
#include "IServer.h"
#include "muduo/net/Endian.h"
#include "muduo/base/Logging.h"
#include <memory>
#include "ConnectionContext.h"
#include "tool.h"
#include <stdlib.h>
#include <memory>

using namespace muduo;
using namespace muduo::net;

const static uint32_t MESSAGE_HEADER_LENGTH = 8;
const static uint32_t LOOP_NUM = 2;

static const uint16_t MSG_TAG = 0x2F11;


IServer::IServer(EventLoop* eventLoop, const InetAddress& listenAddr)
{
	m_loop = eventLoop;
	m_loop->runEvery(1, std::bind(&IServer::onTimer, this));
	//m_turntableTimer = 0;
	m_tcpServer = std::unique_ptr<TcpServer>(new TcpServer(eventLoop, listenAddr, "IServer"));
	m_tcpServer->setConnectionCallback(std::bind(&IServer::onConnection, this, std::placeholders::_1));
	m_tcpServer->setMessageCallback(std::bind(&IServer::onBufferReadable, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void IServer::onTimer()
{

}

void IServer::send(TcpConnection* conn, const StringPiece& message, const uint16_t msgType)
{
	muduo::net::Buffer buf;
	buf.append(message.data(), message.size());
	int32_t len = static_cast<int32_t>(message.size());
	int32_t be32 = sockets::hostToNetwork32(len);
	uint16_t be161 = sockets::hostToNetwork16(msgType);
	uint16_t be162 = sockets::hostToNetwork16(MSG_TAG);

	buf.prepend(&be32, sizeof be32);
	buf.prepend(&be161, sizeof be161);
	buf.prepend(&be162, sizeof be162);
	conn->send(&buf);
}

// tag:0x2F11
// dataType:
// length:lengthOf(data+check)
// CRC:CRC(tag+length+data)
// salt:0xCDEFFEDC
// [tag:4][length:4][data:length][check:4(CRC+salt)]
void IServer::onBufferReadable(const muduo::net::TcpConnectionPtr& conn, Buffer* buf, muduo::Timestamp receiveTime)
{
	while (buf->readableBytes() >= MESSAGE_HEADER_LENGTH)
	{
		uint64_t messageHeader = (uint64_t)buf->peekInt64();
		uint16_t messageTag = sockets::networkToHost16((uint16_t)(messageHeader >> 48));
		uint16_t messageType = sockets::networkToHost16((uint16_t)(messageHeader >> 32));
		uint32_t messageLength = sockets::networkToHost32((uint32_t)messageHeader);
		if (messageTag != 0x2F11 || messageLength == 0 || messageLength > 65535)
		{
			LOG_ERROR << "Invalid Connection";
			conn->shutdown();
			break;
		}
		else if (buf->readableBytes() >= messageLength + MESSAGE_HEADER_LENGTH)
		{
			buf->retrieve(MESSAGE_HEADER_LENGTH);
			std::string str(buf->peek(), messageLength);
			onMessage(conn, messageType, str, receiveTime);
			buf->retrieve(messageLength);
		}
		else
		{
			break;
		}
	}
}

void IServer::start()
{
	m_tcpServer->start();

}

std::string IServer::getAddress()
{
	return m_tcpServer->ipPort();
}


void IServer::send(uint32_t ip, uint16_t port, uint16_t* msg)
{
	char* tmp = new char[16];
	sprintf(tmp, "%d.%d.%d.%d", uint8_t(ip >> 24), uint8_t(ip >> 16), uint8_t(ip >> 8), uint8_t(ip));

}

void IServer::onHeartBeat(muduo::net::TcpConnection* conn)
{
	LOG_INFO << "[onHeartBeat]" << "THREAD ID " << CurrentThread::tid();
	if (!conn->connected())
	{
		return;
	}

	updateHeartBeatTimer(conn, MSGTYPE_NONE);

	string message = "HEART_BEAT";
	muduo::net::Buffer buf;
	buf.append(message.data(), message.size());

	send(conn, message, MSGTYPE_HEART_BEAT_DECTECT);
}

void IServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
	LOG_INFO << "[onConnection]" << "THREAD ID " << CurrentThread::tid();
	LOG_INFO << conn->peerAddress().toIpPort() << (conn->connected() ? " connected" : " disconnected");
	if (conn->connected())
	{
		string connId = conn->peerAddress().toIpPort();
		uint32_t peerIp = stringIp2numIp(conn->peerAddress().toIp());
		uint16_t peerPort = conn->peerAddress().toPort();
		srand(time(0));
		uint64_t connectionId = uint64_t(peerIp) << 32 | uint32_t(peerPort) | (rand() & 0xFFFF);
		ConnectionContext context(connectionId);
		startHeartBeatTimer(get_pointer(conn), context);
	}
	else
	{
		updateHeartBeatTimer(get_pointer(conn), MSGTYPE_DISCONNECTED);
		conn->shutdown();
		//m_connectionTurntable[m_turntableTimer].push_back(conn);
	}
}


// TODO: 多次cancel无效的定时器是否安全
void IServer::startHeartBeatTimer(TcpConnection* conn, ConnectionContext context)
{	
	TimerId timerId = conn->getLoop()->runAfter(HEART_BEAT_TIME, std::bind(&IServer::onHeartBeat, this, conn));
	context.setHeartBeatTimerId(timerId);
	conn->setContext(context);
}

void IServer::updateHeartBeatTimer(TcpConnection* conn, uint16_t msgType)
{
	ConnectionContext context = boost::any_cast<ConnectionContext>(conn->getContext());
	TimerId timerId = context.getHeartBeatTimerId();
	m_loop->cancel(timerId);
	if (msgType != MSGTYPE_NONE)
	{
		if (msgType == MSGTYPE_OTHER_MESSAGE)
		{
			context.resetHeartBeatTimes();
		}
		else if (msgType == MSGTYPE_HEART_BEAT_RESPONSE)
		{
			context.incrHeartBeatTimes();
			if (context.getHeartBeatTimes() > 60)
			{
				conn->shutdown();
				return;
			}
		}
		context.setLossConnectionFlag(CONN_HEART_BEAT_STATUS_OK);
	}
	// 如果下一次定时器到达的时候还没有收到上次心跳的回应，认为客户端已经断开连接
	else if(context.getLossConnectionFlag() != CONN_HEART_BEAT_STATUS_OK)
	{
		conn->shutdown();
	}
	if (conn->connected())
	{
		context.setLossConnectionFlag(CONN_HEART_BEAT_STATUS_WAITING_RESPONSE);
		startHeartBeatTimer(conn, context);
	}
}

void IServer::onMessage(const TcpConnectionPtr& conn, const int16_t messageType, const std::string& message, muduo::Timestamp time)
{
	LOG_INFO <<"[onMessage]"<< "THREAD ID " << CurrentThread::tid();
	LOG_INFO << conn->peerAddress().toIpPort() << " message :" << message;

	switch (messageType)
	{
		case MSGTYPE_OTHER_MESSAGE:
		{
			updateHeartBeatTimer(get_pointer(conn), MSGTYPE_OTHER_MESSAGE);
			break;
		}
		case MSGTYPE_HEART_BEAT_RESPONSE:
		{
			updateHeartBeatTimer(get_pointer(conn), MSGTYPE_HEART_BEAT_RESPONSE);
			break;
		}
		default:
		{
			conn->shutdown();
		}
	}


	//send(conn.get(), StringPiece(message));
}


int main()
{
	EventLoop loop;
	InetAddress address(50000);
	IServer iserver(&loop, address);
	iserver.start();
	loop.loop();


}