#ifndef _HEART_BEAT_H_
#define _HEART_BEAT_H_

#include<stdint.h>
#include<stdio.h>
//#include "muduo/net/EventLoop.h"
#include "muduo/net/TimerId.h"

static const uint16_t MSGTYPE_NONE = 0x0000;
static const uint16_t MSGTYPE_HEART_BEAT_DECTECT = 0x0001;
static const uint16_t MSGTYPE_HEART_BEAT_RESPONSE = 0x0002;
static const uint16_t MSGTYPE_OTHER_MESSAGE = 0x0003;
static const uint16_t MSGTYPE_DISCONNECTED = 0x0004;

static const uint8_t CONN_HEART_BEAT_STATUS_OK = 1;
static const uint8_t CONN_HEART_BEAT_STATUS_WAITING_RESPONSE = 2;


const static uint32_t HEART_BEAT_TIME = 10;

class ConnectionContext
{
public:
	ConnectionContext(uint64_t id);
	muduo::net::TimerId getHeartBeatTimerId();
	void setHeartBeatTimerId(muduo::net::TimerId timerId);
	uint64_t getConnectionId();
	void setConnectionId(uint64_t& connectionId);
	uint8_t getLossConnectionFlag();
	void setLossConnectionFlag(const uint8_t& lossConnectionFlag);
	void incrHeartBeatTimes();
	uint8_t getHeartBeatTimes();
	void resetHeartBeatTimes();
private:
	muduo::net::TimerId m_heartBeatTimerId;
	uint64_t m_connectionId;
	uint8_t m_lossConnectionFlag;
	uint8_t m_onHeartBeatTimes;
};



#endif
