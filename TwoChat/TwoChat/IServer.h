#ifndef _IServer_H_
#define _IServer_H_

#include "muduo/net/TcpConnection.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/InetAddress.h"
#include "ConnectionContext.h"


class IServer
{
public:
	explicit IServer(muduo::net::EventLoop* eventLoop, const muduo::net::InetAddress& listenAddr);
	void start();
	std::string getAddress();
	void send(uint32_t ip, uint16_t port, uint16_t* msg);
	void onConnection(const muduo::net::TcpConnectionPtr& conn);
	void onBufferReadable(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
	void onMessage(const muduo::net::TcpConnectionPtr& conn, const int16_t messageType, const std::string& message, muduo::Timestamp time);
	void onTimer();
	void onHeartBeat(muduo::net::TcpConnection* conn);
	void updateHeartBeatTimer(muduo::net::TcpConnection* conn, uint16_t msgType);
	void startHeartBeatTimer(muduo::net::TcpConnection* conn, ConnectionContext context);
private:

	//std::array<std::vector<muduo::net::TcpConnectionPtr>, kTurnTableSize> m_connectionTurntable;
	//int32_t m_turntableTimer;
	std::unique_ptr<muduo::net::TcpServer> m_tcpServer;
	static void send(muduo::net::TcpConnection* conn, const muduo::StringPiece& message, const uint16_t msgType);
	muduo::net::EventLoop* m_loop;
};

#endif // !_IServer_H_

