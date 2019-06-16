#include "RedisAsync.h"
#include<cstdio>
#include<iostream>
#include<thread>
#include <chrono>
using namespace std;

bool RedisAsyncClient::connect()
{
	// async connect to Redis
	m_redisContext = redisAsyncConnect(m_redisServerAddress.toIp().c_str(), m_redisServerAddress.toPort());
	if (m_redisContext == NULL)
	{
		printf("Redis Connect ERR!!\n");
		return false;
	}
	printf("Redis Connected!!\n");
	// set redis context callbacks
	m_redisContext->ev.data = this;
	m_redisContext->ev.addRead = [](void* data)
	{
		RedisAsyncClient* client = static_cast<RedisAsyncClient*>(data);
		client->m_channel->enableReading();
	};
	m_redisContext->ev.addWrite = [](void* data)
	{
		RedisAsyncClient* client = static_cast<RedisAsyncClient*>(data);
		client->m_channel->enableWriting();
	};
	m_redisContext->ev.delRead = [](void* data)
	{
		RedisAsyncClient* client = static_cast<RedisAsyncClient*>(data);
		client->m_channel->disableReading();
	};
	m_redisContext->ev.delWrite = [](void* data)
	{
		RedisAsyncClient* client = static_cast<RedisAsyncClient*>(data);
		client->m_channel->disableWriting();
	};
	m_redisContext->ev.cleanup = [](void* data)
	{
		RedisAsyncClient* client = static_cast<RedisAsyncClient*>(data);
		client->m_channel->disableAll();
		client->m_channel->remove();
		client->m_channel.reset();
	};
	/*
	redisAsyncSetConnectCallback(m_redisContext, [](const redisAsyncContext * context, int status)
	{
		RedisAsyncClient *client = static_cast<RedisAsyncClient *>(context->ev.data);
		if (client->ConnectedCallback)
		{
			(client->ConnectedCallback)(client, status);
		}
	});
	*/
	redisAsyncSetDisconnectCallback(m_redisContext, [](const redisAsyncContext * context, int status)
	{
		RedisAsyncClient* client = static_cast<RedisAsyncClient*>(context->ev.data);
		//RedisAsyncClient* client1 = this;
		//if (client == client1)
			client->DisconnectedCallback(context, status);
	});

	// init channel
	m_channel.reset(new muduo::net::Channel(m_loop, m_redisContext->c.fd));
	m_channel->setCloseCallback(std::bind(&RedisAsyncClient::tcpDisConnection, this));
	m_channel->setReadCallback(std::bind(&RedisAsyncClient::FdReadable, this, std::placeholders::_1));
	m_channel->setWriteCallback(std::bind(&RedisAsyncClient::FdWritable, this));
	m_channel->setErrorCallback(std::bind(&RedisAsyncClient::FdError, this));
	return true;
}

void RedisAsyncClient::start()
{
	printf("Redis Start!!");
	m_loop->loop();
}

void RedisAsyncClient::DisconnectedCallback(const struct redisAsyncContext* c, int status)
{
	if (status != REDIS_OK) {
		printf("Error: %s\n", c->errstr);
	}
	redisAsyncFree(m_redisContext);
	printf("Reconnecting to Redis……\n");
	connect();
}
//void ConnectedCallback(const struct redisAsyncContext*, int status);
void RedisAsyncClient::tcpDisConnection()
{
	printf("redis tcp disconnected!\n");
	redisAsyncFree(m_redisContext);
	printf("Reconnecting to Redis……\n");
	connect();
}
void RedisAsyncClient::FdReadable(muduo::Timestamp t)
{
	//通知hiredis可读
	redisAsyncHandleRead(m_redisContext);
}
void RedisAsyncClient::FdWritable()
{
	//通知hiredis可写
	redisAsyncHandleWrite(m_redisContext);
}

void RedisAsyncClient::FdError()
{
	// 重置redis连接
	DisconnectedCallback(m_redisContext, REDIS_ERR);
}

void RedisAsyncClient::HandleReply(redisAsyncContext * c, void* treply, void* priv)
{
	
	struct redisReply* reply = (struct redisReply*)treply;
	switch (reply->type)
	{
	case REDIS_REPLY_STRING:
	{
		std::cout << reply->str << std::endl;
		break;
	}
	case REDIS_REPLY_ARRAY:
	{
		for (size_t i = 0; i < reply->elements; ++i)
		{
			HandleReply(c, (reply->element)[i], priv);
		}
		break;
	}
	case REDIS_REPLY_INTEGER:
	{
		std::cout << reply->integer << std::endl;
	}
	break;

	case REDIS_REPLY_NIL:
	{
		break;
	}
	case REDIS_REPLY_STATUS:
	{
		std::cout << reply->str << std::endl;
		break;

	}
	/*	case REDIS_REPLY_ERROR:
		{
			std::cout << reply
		}*/
	default:
		break;
	}
}

int RedisAsyncClient::ExecuteCommand(redisCallbackFn* fn, const char *cmd, ...)
{
	va_list args;
	va_start(args, cmd);
	int ret = redisvAsyncCommand(m_redisContext, HandleReply, nullptr, cmd, args);
	va_end(args);
	return ret;
}

#define _REDIS_ASYNC_MAIN_
#ifdef _REDIS_ASYNC_MAIN_
muduo::net::EventLoop loop;
RedisAsyncClient client(&loop, { "localhost",6379 });
void test()
{
	std::this_thread::sleep_for(std::chrono::seconds(5));
	client.ExecuteCommand(NULL, "SET %s %s", "suwenhao", "199609291");
	client.ExecuteCommand(NULL, "SET %s %s", "suwenhao1", "199609292");
	client.ExecuteCommand(NULL, "SET %s %s", "suwenhao2", "199609293");
	client.ExecuteCommand(NULL, "SET %s %s", "suwenhao3", "199609294");
	client.ExecuteCommand(NULL, "SET %s %s", "suwenhao4", "199609295");
	client.ExecuteCommand(NULL, "SET %s %s", "suwenhao5", "199609296");

	client.ExecuteCommand(NULL, "GET %s", "suwenhao");
	client.ExecuteCommand(NULL, "GET %s", "suwenhao1");
	client.ExecuteCommand(NULL, "GET %s", "suwenhao2");
	client.ExecuteCommand(NULL, "GET %s", "suwenhao3");
	client.ExecuteCommand(NULL, "GET %s", "suwenhao4");
	client.ExecuteCommand(NULL, "GET %s", "suwenhao5");

	client.ExecuteCommand(NULL, "SET %s %s", "suwenhao", "19960929");

	client.ExecuteCommand(NULL, "GET %s", "suwenhao");
}
int main()
{
	client.connect();
	loop.runAfter(5,test);
	client.start();
}
#endif