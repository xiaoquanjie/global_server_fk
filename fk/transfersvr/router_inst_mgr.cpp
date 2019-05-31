#include "transfersvr/router_inst_mgr.h"
#include "protolib/src/svr_base.pb.h"
#include "slience/base/logger.hpp"

int RouterInstanceMgr::LoginInstance(unsigned int server_type, int instance_id, base::s_int64_t fd) {
	if (server_type != proto::SVR_TYPE_ROUTER) {
		return -1;
	}

	auto& instid_index = _info_container.get<RouterInstInfo::tag_inst_id>();
	auto iter = instid_index.find(instance_id);
	if (iter == instid_index.end()) {
		RouterInstInfo info;
		info.fd = fd;
		info.inst_id = instance_id;
		if (!instid_index.insert(info).second) {
			LogError("insert instance fail, instance_id=" << instance_id << " server_type=" << server_type);
			return -1;
		}
	}
	else {
		instid_index.modify(iter, FuncModifyRouterInstInfo(fd, true));
	}

	LogInfo("login router instance: server_type=" << server_type << " instance_id=" << instance_id << " fd=" << fd);
	return 0;
}

void RouterInstanceMgr::LogoutInstance(base::s_int64_t fd) {
	if (fd == 0) {
		return;
	}

	auto& fd_index = _info_container.get<RouterInstInfo::tag_fd>();
	auto iter = fd_index.find(fd);
	if (iter == fd_index.end()) {
		return;
	}

	// fd要清0
	LogInfo("logout router instance: server_type=" << proto::SVR_TYPE_ROUTER << " instance_id=" << iter->inst_id << " fd=" << fd);
	fd_index.modify(iter, FuncModifyRouterInstInfo(0, false));
}

base::s_int64_t RouterInstanceMgr::GetRouterFd(base::s_uint64_t uid) {
	auto& instid_index = _info_container.get<RouterInstInfo::tag_inst_id>();
	
	// 顺序应该是按instid由于到大的
	std::vector< base::s_int64_t> fd_vec;
	for (auto iter = instid_index.begin(); iter != instid_index.end(); ++iter) {
		fd_vec.push_back(iter->fd);
	}

	if (fd_vec.empty()) {
		LogError("can't find router");
		return 0;
	}
	else {
		int r = uid % fd_vec.size();
		return fd_vec[r];
	}
}