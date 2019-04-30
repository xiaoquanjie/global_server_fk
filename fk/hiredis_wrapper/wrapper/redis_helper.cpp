#include "redis_helper.h"

BaseRedisCmd::BaseRedisCmd() {

}

BaseRedisCmd::BaseRedisCmd(const std::vector<std::string>& l) {
	cmd = l;
}

std::string BaseRedisCmd::GetCmd() const {
	if (cmd.size()) {
		return cmd[0];
	}
	else {
		return "Emtpy Cmd";
	}
}

ExpireRedisCmd::ExpireRedisCmd(const char* key, time_t expire) {
	cmd.push_back("EXPIRE");
	cmd.push_back(key);
	cmd.push_back(std::to_string(expire));
}

DelRedisCmd::DelRedisCmd(std::initializer_list<const char*> l) {
	cmd.push_back("DEL");
	for (auto iter = l.begin(); iter != l.end(); ++iter) {
		cmd.push_back(*iter);
	}
}

DelRedisCmd::DelRedisCmd(const std::vector<const char*>& l) {
	cmd.push_back("DEL");
	for (auto iter = l.begin(); iter != l.end(); ++iter) {
		cmd.push_back(*iter);
	}
}

SetRedisCmd::SetRedisCmd(const char* key, const char* value) {
	cmd.push_back("SET");
	cmd.push_back(key);
	cmd.push_back(value);
}

SetNxRedisCmd::SetNxRedisCmd(const char* key, const char* value) {
	cmd.push_back("SETNX");
	cmd.push_back(key);
	cmd.push_back(value);
}

SetExRedisCmd::SetExRedisCmd(const char* key, const char* value, time_t expire) {
	cmd.push_back("SETEX");
	cmd.push_back(key);
	cmd.push_back(std::to_string(expire));
	cmd.push_back(value);
}

GetRedisCmd::GetRedisCmd(const char* key) {
	cmd.push_back("GET");
	cmd.push_back(key);
}

IncrbyRedisCmd::IncrbyRedisCmd(const char* key, int step) {
	cmd.push_back("INCRBY");
	cmd.push_back(key);
	cmd.push_back(std::to_string(step));
}

DecrbyRedisCmd::DecrbyRedisCmd(const char* key, int step) {
	cmd.push_back("DECRBY");
	cmd.push_back(key);
	cmd.push_back(std::to_string(step));
}

StrlenRedisCmd::StrlenRedisCmd(const char* key) {
	cmd.push_back("STRLEN");
	cmd.push_back(key);
}

AppendRedisCmd::AppendRedisCmd(const char* key, const char* value) {
	init(key, value, strlen(value));
}

AppendRedisCmd::AppendRedisCmd(const char* key, const char* value, unsigned int len) {
	init(key, value, len);
}

void AppendRedisCmd::init(const char* key, const char* value, unsigned int len) {
	cmd.push_back("APPEND");
	cmd.push_back(key);
	cmd.push_back(std::string(value, len));
}

SetRangeRedisCmd::SetRangeRedisCmd(const char* key, int beg_idx, const char* value) {
	cmd.push_back("SETRANGE");
	cmd.push_back(key);
	cmd.push_back(std::to_string(beg_idx));
	cmd.push_back(value);
}

GetRangeRediCmd::GetRangeRediCmd(const char* key, int beg_idx, int end_idx) {
	cmd.push_back("GETRANGE");
	cmd.push_back(key);
	cmd.push_back(std::to_string(beg_idx));
	cmd.push_back(std::to_string(end_idx));
}

SetbitRedisCmd::SetbitRedisCmd(const char* key, unsigned int offset, int value) {
	cmd.push_back("SETBIT");
	cmd.push_back(key);
	cmd.push_back(std::to_string(offset));
	cmd.push_back(std::to_string(value));
}

GetbitRedisCmd::GetbitRedisCmd(const char* key, unsigned int offset) {
	cmd.push_back("GETBIT");
	cmd.push_back(key);
	cmd.push_back(std::to_string(offset));
}

/////////////////////////////////////////////////////////////////////////////////////

RedisReplyParser::RedisReplyParser(redisReply* reply) {
	_reply = reply;
}

RedisReplyParser::~RedisReplyParser() {
	if (_reply) {
		freeReplyObject(_reply);
	}
}

void RedisReplyParser::Reset(redisReply* reply) {
	if (_reply) {
		freeReplyObject(_reply);
	}
	_reply = reply;
}

void RedisReplyParser::GetInteger(long long& value) {
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
		if (_reply->type != REDIS_REPLY_INTEGER) {
			error = RedisException(M_ERR_NOT_DEFINED);
			break;
		}
		value = _reply->integer;
	} while (false);
	if (!error.Empty())
		throw error;
}

void RedisReplyParser::GetString(std::string& value) {
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
		if (_reply->type != REDIS_REPLY_STRING) {
			error = RedisException(M_ERR_NOT_DEFINED);
			break;
		}
		value.clear();
		value.append(_reply->str, _reply->len);
	} while (false);

	if (!error.Empty())
		throw error;
}

void RedisReplyParser::GetString(char* value, unsigned int len) {
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
		if (_reply->type != REDIS_REPLY_STRING) {
			error = RedisException(M_ERR_NOT_DEFINED);
			break;
		}

		if (len > (unsigned int)_reply->len)
			len = (unsigned int)_reply->len;
		memcpy(value, _reply->str, len);
	} while (false);

	if (!error.Empty())
		throw error;
}

void RedisReplyParser::GetOk(bool& value) {
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
		if (_reply->type != REDIS_REPLY_STATUS) {
			error = RedisException(M_ERR_NOT_DEFINED);
			break;
		}
		if (strcasecmp(_reply->str, "OK") != 0) {
			value = false;
		}
		else {
			value = true;
		}
	} while (false);

	if (!error.Empty())
		throw error;
}
