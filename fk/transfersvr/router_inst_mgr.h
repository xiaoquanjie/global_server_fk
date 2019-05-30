#pragma once
/*
 * 管理所有连接上来的routersvr
*/

#include "slience/base/singletion.hpp"
#include "transfersvr/router_instance_info.h"

class RouterInstanceMgr {
public:
	int LoginInstance(unsigned int server_type, int instance_id, base::s_int64_t fd);

	void LogoutInstance(base::s_int64_t fd);

	// 寻找出路由
	base::s_int64_t GetRouterFd(base::s_uint64_t uid);

private:
	RouterInstInfoContainer _info_container;
};

#ifndef RouterInstanceMgrSql
#define RouterInstanceMgrSql base::singleton<RouterInstanceMgr>::mutable_instance()
#endif