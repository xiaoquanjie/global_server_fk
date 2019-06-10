#include "redis_connection.hpp"
#include "redis_pool.hpp"
#include "redis_helper.h"

void RedisConnection::set(const char* key, const std::string& value) {
	redisReply* reply = this->Command(SetRedisCmd(key, value.c_str()));
	RedisReplyParser parser(reply);

	bool ok = false;
	parser.GetOk(ok);
	if (!ok) {
		throw RedisException(reply->str);
	}
}

bool RedisConnection::setnx(const char* key, const std::string& value) {
	redisReply* reply = this->Command(SetNxRedisCmd(key, value.c_str()));
	RedisReplyParser parser(reply);

	long long v = 0;
	parser.GetInteger(v);
	return (v == 1 ? true : false);
}

void RedisConnection::setex(const char* key, const std::string& value, time_t expire) {
	redisReply* reply = this->Command(SetExRedisCmd(key, value.c_str(), expire));
	RedisReplyParser parser(reply);

	bool ok = false;
	parser.GetOk(ok);
	if (!ok) {
		throw RedisException(reply->str);
	}
}

//template<typename T>
//void RedisConnection::get(const char* key, T& value) {
//	std::string v;
//	get(key, v);
//	std::istringstream iss(v);
//	iss >> value;
//}

void RedisConnection::get(const char* key, std::string& value) {
	RedisReplyParser parser(this->Command(GetRedisCmd(key)));
	parser.GetString(value);
}

void RedisConnection::get(const char* key, char* value, unsigned int len)
{
	RedisReplyParser parser(this->Command(GetRedisCmd(key)));
	parser.GetString(value, len);
}

void RedisConnection::incrby(const char* key, int step) {
	int value = 0;
	incrby(key, step, value);
}

//template<typename T>
//void RedisConnection::incrby(const char* key, int step, T& new_value) {
//	RedisReplyParser parser(this->Command(IncrbyRedisCmd(key, step)));
//	long long v = 0;
//	parser.GetInteger(v);
//	new_value = (T)v;
//}

//template<typename T>
//void RedisConnection::decrby(const char* key, int step, T& new_value) {
//	RedisReplyParser parser(this->Command(DecrbyRedisCmd(key, step)));
//	long long v = 0;
//	parser.GetInteger(v);
//	new_value = (T)v;
//}

void RedisConnection::decrby(const char* key, int step) {
	int value = 0;
	decrby(key, step, value);
}

int RedisConnection::strlen(const char* key) {
	RedisReplyParser parser(this->Command(StrlenRedisCmd(key)));
	
	long long v = 0;
	parser.GetInteger(v);
	return (int)v;
}

int RedisConnection::append(const char* key, const std::string& app_value) {
	RedisReplyParser parser(this->Command(AppendRedisCmd(key, app_value.c_str())));

	long long v = 0;
	parser.GetInteger(v);
	return (int)v;
}

int RedisConnection::append(const char* key, const char* value, unsigned int len) {
	RedisReplyParser parser(this->Command(AppendRedisCmd(key, value, len)));

	long long v = 0;
	parser.GetInteger(v);
	return (int)v;
}

int RedisConnection::setrange(const char* key, int beg_idx, const char* value, unsigned int len) {
	return setrange(key, beg_idx, std::string(value, len));
}

int RedisConnection::setrange(const char* key, int beg_idx, const std::string& value) {
	RedisReplyParser parser(this->Command(SetRangeRedisCmd(key, beg_idx, value.c_str())));

	long long v = 0;
	parser.GetInteger(v);
	return (int)v;
}

void RedisConnection::getrange(const char* key, int beg_idx, int end_idx, std::string& value) {
	RedisReplyParser parser(this->Command(GetRangeRediCmd(key, beg_idx, end_idx)));
	parser.GetString(value);
}

int RedisConnection::setb(const char* key, unsigned int offset, int value) {
	RedisReplyParser parser(this->Command(SetbitRedisCmd(key, offset, value)));

	long long v = 0;
	parser.GetInteger(v);
	return (int)v;
}

int RedisConnection::getbit(const char* key, unsigned int offset) {
	RedisReplyParser parser(this->Command(GetbitRedisCmd(key, offset)));

	long long v = 0;
	parser.GetInteger(v);
	return (int)v;
}
