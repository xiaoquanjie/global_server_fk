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

int SeverInstanceMgr::LoginInstance(unsigned int server_type, int instance_id, base::s_int64_t fd) {
	const config::Policy* policy = GetPolicy(server_type);
	if (!policy || instance_id ==0) {
		LogError("login instance fail, server_type:" << server_type << "instance_id" << instance_id);
		return -1;
	}
	
	base::s_int32_t basic_id = SvrInstInfo::CalcTypeId(server_type, 0);
	auto& type_inst_index = _info_container.get<SvrInstInfo::tag_type_inst>();
	auto basic_iter = type_inst_index.find(basic_id);
	if (basic_iter == type_inst_index.end()) {
		SvrInstInfo info;
		info.fd = 0;
		info.type_inst = basic_id;
		info.inst_id = 0;
		info.online = false;
		info.info.reset(new InstContainer);
		info.info->policy = policy->policy();
		info.info->svr_type = server_type;
		auto p = type_inst_index.insert(info);
		if (!p.second) {
			LogError("insert instance fail, basic_id=" << basic_id << " server_type=" << server_type);
			return -1;
		}
		basic_iter = p.first;
	}

	base::s_int32_t type_inst = SvrInstInfo::CalcTypeId(server_type, instance_id);
	auto iter = type_inst_index.find(type_inst);
	if (iter == type_inst_index.end()) {
		SvrInstInfo svr_info;
		svr_info.fd = fd;
		svr_info.type_inst = type_inst;
		svr_info.inst_id = instance_id;
		svr_info.online = true;
		svr_info.info = basic_iter->info;
		svr_info.info->inst_set.insert(instance_id);
		if (!type_inst_index.insert(svr_info).second) {
			LogError("add instance fail, server_type:" << server_type << "instance_id" << instance_id);
			return -1;
		}
	}
	else {
		type_inst_index.modify(iter, FuncModifySvrInstInfo(fd, true));
	}
	
	LogInfo("login server instance: server_type=" << server_type << " instance_id=" << instance_id << " fd=" << fd);
	return 0;
}

void SeverInstanceMgr::LogoutInstance(base::s_int64_t fd) {
	if (fd == 0) {
		return;
	}

	auto& fd_index = _info_container.get<SvrInstInfo::tag_fd>();
	auto iter = fd_index.find(fd);
	if (iter == fd_index.end()) {
		return;
	}


	// 暂时不从_type_id_map里删除
	LogInfo("logout server instance: server_type=" << iter->info->svr_type << " instance_id=" << iter->inst_id << " fd=" << fd);
	fd_index.modify(iter, FuncModifySvrInstInfo(fd, false));
}

base::s_int64_t SeverInstanceMgr::GetFdByTypeId(unsigned int server_type, int instance_id) {
	base::s_int32_t id = SvrInstInfo::CalcTypeId(server_type, instance_id);
	auto& type_inst_index = _info_container.get<SvrInstInfo::tag_type_inst>();
	auto iter = type_inst_index.find(id);
	if (iter == type_inst_index.end()) {
		return 0;
	}
	else {
		return iter->fd;
	}
}

void SeverInstanceMgr::RouterPolicy(
	base::s_uint64_t uid, unsigned int server_type, bool is_broadcast, std::vector<int>& inst_vec) {
	base::s_int32_t id = SvrInstInfo::CalcTypeId(server_type, 0);
	auto& type_inst_index = _info_container.get<SvrInstInfo::tag_type_inst>();
	auto iter = type_inst_index.find(id);
	if (iter == type_inst_index.end()) {
		LogError("can't find instance, server_type:" << server_type);
		return;
	}

	std::vector<int> all_inst;
	for (auto id : iter->info->inst_set) {
		all_inst.push_back(id);
	}
	if (all_inst.empty()) {
		LogError("instance vector is empty, server_type:" << server_type);
		return;
	}

	if (is_broadcast) {
		all_inst.swap(inst_vec);
		return;
	}

	switch (iter->info->policy) {
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