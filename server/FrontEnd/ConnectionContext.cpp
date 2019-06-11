
#include "ConnectionContext.h"

using namespace muduo;
using namespace muduo::net;


ConnectionContext::ConnectionContext(uint64_t id)
{
	m_connectionId = id;
	m_lossConnectionFlag = CONN_HEART_BEAT_STATUS_OK;
	m_onHeartBeatTimes = 0;
}

TimerId ConnectionContext::getHeartBeatTimerId()
{
	return m_heartBeatTimerId;
}

void ConnectionContext::setHeartBeatTimerId(TimerId timerId)
{
	m_heartBeatTimerId = timerId;
}

uint64_t ConnectionContext::getConnectionId()
{
	return m_connectionId;
}

void ConnectionContext::setConnectionId(uint64_t& connectionId)
{
	m_connectionId = connectionId;
}

uint8_t ConnectionContext::getLossConnectionFlag()
{
	return m_lossConnectionFlag;
}

void ConnectionContext::setLossConnectionFlag(const uint8_t& lossConnectionFlag)
{
	m_lossConnectionFlag = lossConnectionFlag;
}

void ConnectionContext::incrHeartBeatTimes()
{
	++m_onHeartBeatTimes;
}

uint8_t ConnectionContext::getHeartBeatTimes()
{
	return m_onHeartBeatTimes;
}

void ConnectionContext::resetHeartBeatTimes()
{
	m_onHeartBeatTimes = 0;
}
