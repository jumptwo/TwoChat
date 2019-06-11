#ifndef _TWOCHAT_MESSAGE_TYPE_H_
#define _TWOCHAT_MESSAGE_TYPE_H_

#include<stdint.h>
#include<string>

// 不安全函数，外部保证输出参数的正确性
uint32_t stringIp2numIp(std::string ip)
{
	uint32_t index = 0;
	int a[4] = { 0,0,0,0 };
	for (uint32_t i = 0; i < ip.length(); i++)
	{
		if (ip[i] == '.')
		{
			index++;
		}
		else
		{
			a[index] = a[index] * 10 + ip[i] - '0';
		}
	}
	return (a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3];
}

void printMessageInHex(uint8_t* buf, uint32_t len)
{
	if (len == 0 || buf == nullptr)
	{
		LOG_ERROR << "printMessageInHex ERR";
		LOG_ERROR << muduo::CurrentThread::stackTrace(true);
		return;
	}
	std::string recv = "";
	for (uint32_t i = 0; i < len; i++)
	{
		char s[20];
		sprintf(s, "%.2x ", *(buf + i));
		recv.append(s);
	}
	LOG_INFO << recv;
}


#endif
