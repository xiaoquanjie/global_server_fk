#ifndef M_REDIS_OTHERS_INCLUDE
#define M_REDIS_OTHERS_INCLUDE

#include "redis_connection.hpp"
#include "redis_pool.hpp"
#include "redis_helper.h"

redisReply* RedisConnection::Command(const BaseRedisCmd& cmd) {
	M_CHECK_REDIS_CONTEXT(_context);
	std::vector<const char *> argv(cmd.cmd.size());
	std::vector<size_t> argvlen(cmd.cmd.size());
	for (size_t idx = 0; idx < cmd.cmd.size(); ++idx) {
		argv[idx] = cmd.cmd[idx].c_str();
		argvlen[idx] = cmd.cmd[idx].length();
	}
	redisReply* reply = (redisReply*)wredisCommandArgv(*this, cmd.cmd.size(), &(argv[0]), &(argvlen[0]));
	if (!reply)
		M_CLOSE_CONNECTION(this);

	return reply;
}

bool RedisConnection::expire(const char* key, time_t expire)
{
	redisReply* reply = this->Command(ExpireRedisCmd(key, expire));
	RedisReplyParser parser(reply);
	long long value = 0;
	parser.GetInteger(value);
	return (value == 1 ? true : false);
}

int RedisConnection::del(const char* key) {
	std::vector<const char*> vec;
	vec.push_back(key);
	return dels(vec);
}

int RedisConnection::dels(const std::vector<const char*>& l) {
	redisReply* reply = this->Command(DelRedisCmd(l));
	RedisReplyParser parser(reply);

	long long value = 0;
	parser.GetInteger(value);
	return (int)value;
}

#endif