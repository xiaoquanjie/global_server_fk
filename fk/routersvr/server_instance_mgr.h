#pragma once

#include "slience/base/singletion.hpp"
#include "protolib/src/routersvr_config.pb.h"
#include "routersvr/server_instance_info.h"
#include <set>
#include <unordered_map>

class SeverInstanceMgr {
public:
	SeverInstanceMgr();

	int Init(const config::RouterSvrConfig* conf);

	int LoginInstance(unsigned int server_type, int instance_id, base::s_int64_t fd = 0);

	void LogoutInstance(base::s_int64_t fd);

	base::s_int64_t GetFdByTypeId(unsigned int server_type, int instance_id);

	void RouterPolicy(base::s_uint64_t uid, unsigned int server_type, bool is_broadcast, std::vector<int>& inst_vec);

protected:
	const config::Policy* GetPolicy(unsigned int server_type);

private:
	const config::RouterSvrConfig* _config;
	SvrInstInfoContainer _info_container;
};

#ifndef SeverInstanceMgrSgl
#define SeverInstanceMgrSgl base::singleton<SeverInstanceMgr>::mutable_instance()
#endif
