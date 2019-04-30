#pragma once

#include "slience/base/config.hpp"
#include <vector>

struct MysqlReq {
	enum {
		E_QUERY = 0,
		E_EXECUTE = 1,
	};
	int type;
	base::s_uint32_t trans_id;
	std::string url;	// 格式: host:port:user:passwd:db
	std::string sql;
};

struct MysqlRsp {
	std::string code_what;
	base::s_uint32_t trans_id;
	int err_code;
	int affected_rows;
	void* mysql_res;
	bool is_dup_entry;
};

/////////////////////////////////////////////////////////////////////////////

struct RedisReq {
	base::s_uint32_t trans_id;
	std::string url;	// 格式: host:port:user:passwd:db
	std::vector<std::string> cmd;
};

struct RedisRsp {
	base::s_uint32_t trans_id;
	int err_code;
	std::string code_what;
	void* reply;
};
