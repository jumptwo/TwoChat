
#include "RedisModule.h"

int main()
{
	RedisModule redisModule;
	redisModule.connect();
	redisModule.setString("hello", "world");


}