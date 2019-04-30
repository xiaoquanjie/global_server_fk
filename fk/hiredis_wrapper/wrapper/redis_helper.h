#pragma once

#include "redis_wrapper_config.hpp"

struct BaseRedisCmd {
	std::vector<std::string> cmd;

	BaseRedisCmd();

	BaseRedisCmd(const std::vector<std::string>& l);

	std::string GetCmd() const;
};

struct ExpireRedisCmd : public BaseRedisCmd {
	ExpireRedisCmd(const char* key, time_t expire);
};

struct DelRedisCmd : public BaseRedisCmd {
	DelRedisCmd(std::initializer_list<const char*> l);

	DelRedisCmd(const std::vector<const char*>& l);
};

struct SetRedisCmd : public BaseRedisCmd {
	SetRedisCmd(const char* key, const char* value);
};

struct SetNxRedisCmd : public BaseRedisCmd {
	SetNxRedisCmd(const char* key, const char* value);
};

struct SetExRedisCmd : public BaseRedisCmd {
	SetExRedisCmd(const char* key, const char* value, time_t expire);
};

struct GetRedisCmd : public BaseRedisCmd {
	GetRedisCmd(const char* key);
};

struct IncrbyRedisCmd : public BaseRedisCmd {
	IncrbyRedisCmd(const char* key, int step);
};

struct DecrbyRedisCmd : public BaseRedisCmd {
	DecrbyRedisCmd(const char* key, int step);
};

struct StrlenRedisCmd : public BaseRedisCmd {
	StrlenRedisCmd(const char* key);
};

struct AppendRedisCmd : public BaseRedisCmd {
	AppendRedisCmd(const char* key, const char* value);

	AppendRedisCmd(const char* key, const char* value, unsigned int len);

private:
	void init(const char* key, const char* value, unsigned int len);
};

struct SetRangeRedisCmd : public BaseRedisCmd {
	SetRangeRedisCmd(const char* key, int beg_idx, const char* value);
};

struct GetRangeRediCmd : public BaseRedisCmd {
	GetRangeRediCmd(const char* key, int beg_idx, int end_idx);
};

struct SetbitRedisCmd : public BaseRedisCmd {
	SetbitRedisCmd(const char* key, unsigned int offset, int value);
};

struct GetbitRedisCmd : public BaseRedisCmd {
	GetbitRedisCmd(const char* key, unsigned int offset);
};

/////////////////////////////////////////////////////////////////////////////////////

struct redisReply;

struct RedisReplyParser {
	RedisReplyParser(redisReply* reply);

	~RedisReplyParser();

	void Reset(redisReply* reply);

	void GetInteger(long long& value);

	void GetString(std::string& value);

	void GetString(char* value, unsigned int len);

	template<typename T>
	void GetArray(T& values) {
		RedisException error;
		do {
			if (!_reply) {
				error = RedisException(M_ERR_REDIS_REPLY_NULL);
				break;
			}
			if (_reply->type == REDIS_REPLY_ERROR) {
				error = RedisException(_reply->str);
				break;
			}
			if (_reply->type != REDIS_REPLY_ARRAY) {
				error = RedisException(M_ERR_NOT_DEFINED);
				break;
			}
			
			for (size_t idx = 0; idx < _reply->elements; ++idx) {
				redisReply* ele = _reply->element[idx];
				std::istringstream iss(std::string(ele->str, ele->len));
				typename T::value_type v;
				iss >> v;
				values.push_back(v);
			}

		} while (false);
		if (!error.Empty())
			throw error;
	}

	template<typename T1, typename T2>
	void GetMap(std::map<T1, T2>& values) {
		RedisException error;
		do {
			if (!_reply) {
				error = RedisException(M_ERR_REDIS_REPLY_NULL);
				break;
			}
			if (_reply->type == REDIS_REPLY_ERROR) {
				error = RedisException(_reply->str);
				break;
			}
			if (_reply->type != REDIS_REPLY_ARRAY) {
				error = RedisException(M_ERR_NOT_DEFINED);
				break;
			}

			for (size_t idx = 0; idx < _reply->elements; idx += 2) {
				std::istringstream iss1(std::string(_reply->element[idx]->str, _reply->element[idx]->len));
				std::istringstream iss2(std::string(_reply->element[idx + 1]->str, _reply->element[idx + 1]->len));

				T1 value1;
				iss1 >> value1;
				T2 value2;
				iss2 >> value2;
				values.insert(std::make_pair(value1, value2));
			}

		} while (false);
		if (!error.Empty())
			throw error;
	}

	void GetOk(bool& value);

private:
	redisReply* _reply;
};
