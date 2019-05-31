#include "commonlib/net_handler/router_mgr.h"
#include "slience/base/logger.hpp"
#include "commonlib/net_handler/net_handler.h"
#include <map>
#include "protolib/src/svr_base.pb.h"
#include "protolib/src/cmd.pb.h"

RouterMgr::RouterMgr() {
	_self_server_type = 0;
	_self_instance_id = 0;
	_self_server_zone = 0;
}

int RouterMgr::Init(int self_svr_type,
	int self_inst_id,
	int self_server_zone, 
	const std::string& router_file) {
	_self_server_type = self_svr_type;
	_self_instance_id = self_inst_id;
	_self_server_zone = self_server_zone;
	_router_file = router_file;
	int ret = 0;
	do {
		ret = Reload();
		if (0 != ret) {
			LogError("reload fail");
			break;
		}
	} while (false);
	return ret;
}

int RouterMgr::Reload() {
	if (_router_file.empty()) {
		return 0;
	}

	ServerCfg<config::RouterConfig> tmp_router_config;
	if (0 != tmp_router_config.Parse(_router_file.c_str())) {
		LogError("_router_config.Parse fail: " << _router_file);
		return -1;
	}

	std::set<int> inst_id_set;
	for (int idx = 0; idx < tmp_router_config.Data().router_list_size(); ++idx) {
		int inst_id = tmp_router_config.Data().router_list(idx).inst_id();
		int zone = tmp_router_config.Data().router_list(idx).svr_zone();
		if (zone != SelfServerZone()) {
			continue;
		}
		if (inst_id_set.count(inst_id) > 0) {
			LogError("router inst_id is duplicated: " << inst_id);
			return -1;
		}
		inst_id_set.insert(inst_id);
	}

	if (0 != ConnectRouters(tmp_router_config)) {
		LogError("ConnectRouters fail");
		return -1;
	}
	return 0;
}

void RouterMgr::Tick(const base::timestamp& now) {
	proto::SvrHeatBeat msg;
	msg.set_server_type(SelfSeverType());
	msg.set_instance_id(SelfInstanceId());
	for (auto iter = _router_info_vec.begin();
		iter != _router_info_vec.end(); ++iter) {
		SendMsgByFd(iter->fd,
			proto::CMD::CMD_SVR_HEATBEAT,
			0,
			false,
			SelfServerZone(),
			proto::SVR_TYPE_ROUTER,
			iter->inst_id,
			0,
			0,
			0,
			msg);
	}
}

int RouterMgr::ConnectRouters(ServerCfg<config::RouterConfig>& router_config) {
	// 设计原则是，多连少关
	std::map<int, RouterInfo> tmp_router_info_map;
	for (int idx = 0; idx < router_config.Data().router_list_size(); ++idx) {
		auto& item = router_config.Data().router_list(idx);
		// 只连接相同区的路由
		if (item.svr_zone() != SelfServerZone()) {
			continue;
		}
		if (!ExistRouter(item.listen_ip(), item.listen_port(), item.inst_id())) {
			NetIoHandlerSgl.ConnectOne(item.listen_ip(), item.listen_port(),
				Enum_ConnType_Router, item.inst_id());

			RouterInfo router_info;
			router_info.ip = item.listen_ip();
			router_info.port = item.listen_port();
			router_info.inst_id = item.inst_id();
			router_info.fd = 0;
			tmp_router_info_map[item.inst_id()] = router_info;
		}
	}

	// 已被关闭了的
	for (auto& info : _router_info_vec) {
		bool exist = false;
		for (int idx = 0; idx < router_config.Data().router_list_size(); ++idx) {
			auto& item = router_config.Data().router_list(idx);
			if (info.ip == item.listen_ip()
				&& info.port == item.listen_port()
				&& info.inst_id == item.inst_id()) {
				exist = true;
				break;
			}
		}

		if (exist) {
			tmp_router_info_map[info.inst_id] = info;
		}
		else {
			NetIoHandlerSgl.CloseFd(info.fd);
		}
	}

	_router_info_vec.clear();
	for (auto iter = tmp_router_info_map.begin(); iter != tmp_router_info_map.end();
		++iter) {
		_router_info_vec.push_back(iter->second);
	}
	return 0;
}

bool RouterMgr::ExistRouter(const std::string& ip,
	unsigned int port, 
	base::s_int32_t inst_id) {
	for (auto iter = _router_info_vec.begin(); iter != _router_info_vec.end(); ++iter) {
		if (ip == iter->ip
			&& port == iter->port
			&& inst_id == iter->inst_id) {
			return true;
		}
	}
	return false;
}

int RouterMgr::LoginRouter(const std::string& ip,
	unsigned int port,
	base::s_int32_t inst_id,
	base::s_int64_t fd) {
	for (auto iter = _router_info_vec.begin(); iter != _router_info_vec.end();
		++iter) {
		if (ip == iter->ip
			&& port == iter->port
			&& inst_id == iter->inst_id) {
			iter->fd = fd;
			return 0;
		}
	}
	return -1;
}

int RouterMgr::SendMsg(int cmd, 
	base::s_int64_t userid, 
	bool is_broadcast,
	base::s_uint16_t dst_zone,
	base::s_uint32_t dst_svr_type,
	base::s_uint32_t dst_inst_id,
	base::s_uint32_t src_trans_id,
	base::s_uint32_t dst_trans_id,
	base::s_uint32_t req_random,
	google::protobuf::Message& msg) {
	// 挑选一个路由
	if (_router_info_vec.empty()) {
		LogError("no router to send");
		return -1;
	}
	int r = userid % _router_info_vec.size();
	return SendMsgByFd(_router_info_vec[r].fd, cmd,
		userid, is_broadcast,
		dst_zone, dst_svr_type,
		dst_inst_id, src_trans_id, 
		dst_trans_id, req_random, 
		msg);
}

int RouterMgr::SendMsgByFd(base::s_int64_t fd,
	int cmd,
	base::s_int64_t userid, 
	bool is_broadcast,
	base::s_uint16_t dst_zone, 
	base::s_uint32_t dst_svr_type,
	base::s_uint32_t dst_inst_id,
	base::s_uint32_t src_trans_id, 
	base::s_uint32_t dst_trans_id, 
	base::s_uint32_t req_random,
	google::protobuf::Message& msg) {
	AppHeadFrame frame;
	frame.set_is_broadcast(is_broadcast);
	frame.set_src_zone(SelfServerZone());
	frame.set_dst_zone(dst_zone);
	frame.set_src_svr_type(SelfSeverType());
	frame.set_dst_svr_type(dst_svr_type);
	frame.set_src_inst_id(SelfInstanceId());
	frame.set_dst_inst_id(dst_inst_id);
	frame.set_src_trans_id(src_trans_id);
	frame.set_dst_trans_id(dst_trans_id);
	frame.set_cmd(cmd);
	frame.set_userid(userid);
	frame.set_req_random(req_random);

	std::string data = msg.SerializePartialAsString();
	frame.set_cmd_length(data.length());

	base::Buffer buffer;
	buffer.Write(frame);
	buffer.Write(data.c_str(), data.length());
	if (NetIoHandlerSgl.SendDataByFd(fd, buffer.Data(), buffer.Length())) {
		return 0;
	}
	else {
		return -1;
	}
}

int RouterMgr::SelfSeverType() {
	return _self_server_type;
}

int RouterMgr::SelfInstanceId() {
	return _self_instance_id;
}

int RouterMgr::SelfServerZone() {
	return _self_server_zone;
}