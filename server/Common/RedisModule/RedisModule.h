#ifndef _REDIS_MODULE_H_
#define _REDIS_MODULE_H_

#include "hiredis/hiredis.h"
#include <string>
#include "hiredis/async.h"

#define DEBUG_MODE

class RedisModule
{
public:
	RedisModule(std::string& redisServerIp, uint16_t redisServerPort)
		:redis(nullptr)
	{
		m_redisServerIp = redisServerIp;
		m_redisServerPort = redisServerPort;
	}
	RedisModule()
		:redis(nullptr), m_redisServerIp("localhost"), m_redisServerPort(6379)
	{
	}
	bool connect()
	{
		timeval timeout;
		timeout.tv_usec = 0;
		timeout.tv_sec = 60;
		redisContext* tmpContext = redisConnectWithTimeout(m_redisServerIp.c_str(), m_redisServerPort, timeout);
		if (tmpContext == NULL || tmpContext->err)
		{
			printf("redis connect fail:[%d][%s]\n", tmpContext->err, tmpContext->errstr);
			return false;
		}
		printf("redis connected.\n");
		redis = tmpContext;
		return true;
	}

	bool setString(const std::string& key, const std::string& value)
	{
		if (redis == nullptr)
		{
			return false;
		}
		if (key.length() + value.length() > 2040)
		{
			printf("[RedisModule]key.length + value.length is two big");
			return false;
		}
		char setCommandFormat[] = "SET %s %s";

		redisCommand(redis, setCommandFormat, key.c_str(), value.c_str());
	}

	bool getString(const std::string& key, const std::string& value)
	{
		if (redis == nullptr)
		{
			return false;
		}
		if (key.length() + value.length() > 2040)
		{
			printf("[RedisModule]key.length + value.length is two big");
			return false;
		}
		char setCommandFormat[] = "GET %s %s";

		redisCommand(redis, setCommandFormat, key.c_str(), value.c_str());
	}
private:
	std::string m_redisServerIp;
	uint16_t m_redisServerPort;
	redisContext* redis;

};





#endif