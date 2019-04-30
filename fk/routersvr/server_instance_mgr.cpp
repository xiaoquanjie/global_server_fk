#include "routersvr/server_instance_mgr.h"
#include "slience/base/logger.hpp"

SeverInstanceMgr::SeverInstanceMgr() {
	_config = 0;
}

int SeverInstanceMgr::Init(const config::RouterSvrConfig* conf) {
	if (!conf) {
		LogError("conf is null");
		return -1;
	}
	_config = conf;
	return 0;
}

int SeverInstanceMgr::AddInstance(unsigned int server_type, int instance_id, base::s_int64_t fd) {
	const config::Policy* policy = GetPolicy(server_type);
	if (!policy) {
		LogError("add instance fail, server_type:" << server_type << "instance_id" << instance_id);
		return -1;
	}

	auto iter_type = _type_id_map.find(server_type);
	if (iter_type == _type_id_map.end()) {
		_type_id_map[server_type] = InstanceInfo();
		iter_type = _type_id_map.find(server_type);
	}
	
	iter_type->second.svr_type = server_type;
	iter_type->second.policy = policy->policy();
	iter_type->second.inst_vec.insert(instance_id);
	
	SvrInfo svrinfo;
	svrinfo.svr_type = server_type;
	svrinfo.svr_inst = instance_id;
	_svrinfo_fd_map[svrinfo] = fd;
	_fd_svrinfo_map[fd] = svrinfo;
	LogInfo("add server instance: server_type=" << server_type << " instance_id=" << instance_id << " fd=" << fd);
	return 0;
}

void SeverInstanceMgr::DelInstance(unsigned int server_type, int instance_id) {
	auto iter_type = _type_id_map.find(server_type);
	if (iter_type == _type_id_map.end()) {
		LogError("can't find server instance in _type_id_map: server_type=" << server_type << " instance_id=" << instance_id);
	}
	else {
		iter_type->second.inst_vec.erase(instance_id);
		LogInfo("del server instance: server_type=" << server_type << " instance_id=" << instance_id);
	}

	SvrInfo svrinfo;
	svrinfo.svr_type = server_type;
	svrinfo.svr_inst = instance_id;
	auto iter_svr = _svrinfo_fd_map.find(svrinfo);
	if (iter_svr != _svrinfo_fd_map.end()) {
		LogInfo("del server instance: server_type=" << server_type << " instance_id=" << instance_id << " fd=" << iter_svr->second);
		_fd_svrinfo_map.erase(iter_svr->second);
		_svrinfo_fd_map.erase(iter_svr);
	}
	else {
		LogError("can't find server instance in _svrinfo_fd_map : server_type=" << server_type << " instance_id=" << instance_id);
	}
}

void SeverInstanceMgr::DelInstance(base::s_int64_t fd) {
	auto iter_fd = _fd_svrinfo_map.find(fd);
	if (iter_fd == _fd_svrinfo_map.end()) {
		LogError("can't find fd=" << fd);
		return;
	}

	// 暂时不从_type_id_map里删除
	LogInfo("del server instance: server_type=" << iter_fd->second.svr_type << " instance_id=" << iter_fd->second.svr_inst << " fd=" << fd);
	_svrinfo_fd_map.erase(iter_fd->second);
	_fd_svrinfo_map.erase(iter_fd);
}

base::s_int64_t SeverInstanceMgr::GetFdByTypeId(unsigned int server_type, int instance_id) {
	SvrInfo info;
	info.svr_type = server_type;
	info.svr_inst = instance_id;
	auto iter_svrinfo = _svrinfo_fd_map.find(info);
	if (iter_svrinfo == _svrinfo_fd_map.end()) {
		return 0;
	}
	else {
		return iter_svrinfo->second;
	}
}

SvrInfo* SeverInstanceMgr::GetSvrInfoByFd(base::s_int64_t fd) {
	auto iter_fd = _fd_svrinfo_map.find(fd);
	if (iter_fd == _fd_svrinfo_map.end()) {
		return 0;
	}
	else {
		return &iter_fd->second;
	}
}

void SeverInstanceMgr::RouterPolicy(
	base::s_uint64_t uid, unsigned int server_type, bool is_broadcast, std::vector<int>& inst_vec) {
	auto iter_type = _type_id_map.find(server_type);
	if (iter_type == _type_id_map.end()) {
		LogError("can't find instance, server_type:" << server_type);
		return;
	}

	std::vector<int> all_inst;
	for (auto iter_id = iter_type->second.inst_vec.begin(); iter_id != iter_type->second.inst_vec.end();
		++iter_id) {
		all_inst.push_back(*iter_id);
	}
	if (all_inst.empty()) {
		LogError("instance vector is empty, server_type:" << server_type);
		return;
	}

	if (is_broadcast) {
		all_inst.swap(inst_vec);
		return;
	}

	switch (iter_type->second.policy) {
	case config::POLICY_ROUTER:
		break;
	case config::POLICY_RANDOM:{
		int t = all_inst[0];
		inst_vec.push_back(t);
		break;
	}
	case config::POLICY_MOD: {
		int u = uid % all_inst.size();
		inst_vec.push_back(all_inst[u]);
		break;
	}
	case config::POLICY_BROADCAST: {
		all_inst.swap(inst_vec);
		break;
	}
	}
}

const config::Policy* SeverInstanceMgr::GetPolicy(unsigned int server_type) {
	if (!_config) {
		LogError("_config is null");
		return 0;
	}

	for (int idx = 0; idx < _config->policy_list_size(); ++idx) {
		if (_config->policy_list(idx).svr_type() == server_type) {
			return &_config->policy_list(idx);
		}
	}
	LogError("can't find server_type:" << server_type);
	return 0;
}