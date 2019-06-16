#ifndef _REDIS_ASYNC_H_
#define _REDIS_ASYNC_H_

#include "hiredis/hiredis.h"
#include "hiredis/async.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/Channel.h"
#include <memory>
#include <string>
#include <iostream>

class RedisAsyncClient
{
public:
	RedisAsyncClient(muduo::net::EventLoop* loop, muduo::net::InetAddress redisServerAddress)
		:m_loop(loop), m_redisServerAddress(redisServerAddress)
	{
	}
	~RedisAsyncClient()
	{
		redisAsyncFree(m_redisContext);
	}
private:
	redisAsyncContext* m_redisContext;
	muduo::net::EventLoop* m_loop;
	muduo::net::InetAddress m_redisServerAddress;
	std::unique_ptr<muduo::net::Channel> m_channel;
	void DisconnectedCallback(const struct redisAsyncContext* c, int status);
	//void ConnectedCallback(const struct redisAsyncContext*, int status);
	void tcpDisConnection();
	void FdReadable(muduo::Timestamp t);
	void FdWritable();
	void FdError();
public:
	bool connect();
	void start();
	static void HandleReply(redisAsyncContext * c, void* treply, void*);
	int ExecuteCommand(redisCallbackFn* fn, const char *cmd, ...);
};




#endif