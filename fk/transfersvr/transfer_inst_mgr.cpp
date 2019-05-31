#include "transfersvr/transfer_inst_mgr.h"
#include "commonlib/net_handler/net_handler.h"
#include "protolib/src/svr_base.pb.h"
#include "protolib/src/cmd.pb.h"

TransferInstanceMgr::TransferInstanceMgr() {
	_self_instance_id = 0;
	_self_server_type = 0;
	_self_server_zone = 0;
}

int TransferInstanceMgr::Init(int self_svr_type,
	int self_inst_id,
	int self_server_zone,
	const std::string& transfer_file) {
	_self_server_type = self_svr_type;
	_self_instance_id = self_inst_id;
	_self_server_zone = self_server_zone;
	_transfer_file = transfer_file;

	int ret = Reload();
	if (0 != ret) {
		LogError("reload fail");
	}
	return ret;
}

int TransferInstanceMgr::Reload() {
	if (_transfer_file.empty()) {
		return 0;
	}

	ServerCfg<config::TransferConfig> transfer_config;
	if (0 != transfer_config.Parse(_transfer_file.c_str())) {
		LogError("transfer_config.Parse fail: " << _transfer_file);
		return -1;
	}

	std::map<int, std::set<int>> zone_instid_map;
	for (int idx = 0; idx < transfer_config.Data().transfer_list_size(); ++idx) {
		int zone = transfer_config.Data().transfer_list(idx).svr_zone();
		int inst_id = transfer_config.Data().transfer_list(idx).inst_id();
		if (zone_instid_map[zone].count(inst_id) > 0) {
			LogError("transfer inst_id:" << inst_id << " in zone:" << zone << " is duplicated");
			return -1;
		}

		zone_instid_map[zone].insert(inst_id);
	}

	if (0 != ConnectTransfers(transfer_config)) {
		LogError("ConnectTransfers fail");
		return -1;
	}

	return 0;
}

void TransferInstanceMgr::Tick(const base::timestamp& now) {
	if ((now.second() - _last_snd_time.second()) < 20) {
		return;
	}

	_last_snd_time = now;
	proto::TransferHeatBeat msg;
	msg.set_server_zone(SelfServerZone());
	msg.set_instance_id(SelfInstanceId());
	for (auto iter1 = _zone_transfer_map.begin(); iter1 != _zone_transfer_map.end(); ++iter1) {
		for (auto iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2) {
			if (!iter2->active) {
				SendMsgToTransferByFd(iter2->fd,
					proto::CMD::CMD_TRANSFER_HEATBEAT,
					0,
					iter2->zone,
					proto::SVR_TYPE_TRANSFER,
					iter2->inst_id,
					0,
					0,
					0,
					msg);
			}
		}
	}
}

int TransferInstanceMgr::SelfSeverType() {
	return _self_server_type;
}

int TransferInstanceMgr::SelfInstanceId() {
	return _self_instance_id; 
}

int TransferInstanceMgr::SelfServerZone() {
	return _self_server_zone; 
}

bool TransferInstanceMgr::ExistTransfer(const std::string& ip,
	unsigned int port,
	base::s_int32_t inst_id) {
	for (auto iter1 = _zone_transfer_map.begin(); iter1 != _zone_transfer_map.end(); ++iter1) {
		for (auto iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2) {
			if (ip == iter2->ip && port == iter2->port && inst_id == iter2->inst_id) {
				return true;
			}
		}
	}
	return false;
}

int TransferInstanceMgr::LoginTransfer(const std::string& ip,
	unsigned int port,
	base::s_int32_t inst_id,
	base::s_int64_t fd) {
	for (auto iter1 = _zone_transfer_map.begin(); iter1 != _zone_transfer_map.end(); ++iter1) {
		for (auto iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2) {
			if (iter2->ip == ip && iter2->port == port && iter2->inst_id == inst_id) {
				iter2->online = true;
				iter2->fd = fd;
				break;
			}
		}
	}
	return 0;
}

int TransferInstanceMgr::LoginTransfer(base::s_int32_t server_zone,
	base::s_int32_t inst_id,
	base::s_int64_t fd) {
	for (auto iter1 = _zone_transfer_map.begin(); iter1 != _zone_transfer_map.end(); ++iter1) {
		for (auto iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2) {
			if (iter2->zone == server_zone && iter2->inst_id == inst_id) {
				iter2->online = true;
				iter2->fd = fd;
				break;
			}
		}
	}
	return 0;
}

int TransferInstanceMgr::LogoutTransfer(base::s_int64_t fd) {
	if (fd == 0) {
		return -1;
	}
	for (auto iter1 = _zone_transfer_map.begin(); iter1 != _zone_transfer_map.end(); ++iter1) {
		for (auto iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2) {
			if (iter2->fd == fd) {
				iter2->online = false;
				iter2->fd = 0;
				return 0;
			}
		}
	}
	return -1;
}

int TransferInstanceMgr::SendMsgToTransferByFd(base::s_uint64_t fd,
	int cmd,
	base::s_uint64_t userid,
	base::s_uint16_t dst_zone,
	base::s_uint32_t dst_svr_type,
	base::s_uint32_t dst_inst_id,
	base::s_uint32_t src_trans_id,
	base::s_uint32_t dst_trans_id,
	base::s_uint32_t req_random,
	google::protobuf::Message& msg) {
	_frame.set_is_broadcast(false);
	_frame.set_src_zone(SelfServerZone());
	_frame.set_dst_zone(dst_zone);
	_frame.set_src_svr_type(SelfSeverType());
	_frame.set_dst_svr_type(dst_svr_type);
	_frame.set_src_inst_id(SelfInstanceId());
	_frame.set_dst_inst_id(dst_trans_id);
	_frame.set_src_trans_id(src_trans_id);
	_frame.set_dst_trans_id(dst_trans_id);
	_frame.set_cmd(cmd);
	_frame.set_userid(userid);
	_frame.set_req_random(req_random);

	std::string data = msg.SerializePartialAsString();
	_frame.set_cmd_length(data.length());

	_buffer.Write(_frame);
	_buffer.Write(data.c_str(), data.length());
	if (NetIoHandlerSgl.SendDataByFd(fd, _buffer.Data(), _buffer.Length())) {
		return 0;
	}
	else {
		return -1;
	}
}

int TransferInstanceMgr::ConnectTransfers(ServerCfg<config::TransferConfig>& transfer_config) {
	// 设计原则是，多连少关
	std::unordered_map <base::s_int32_t, std::unordered_map<base::s_int32_t, TransferSvrInfo>> tmp_zone_transfer_map;
	int self_seq = SelfServerZone() * 1000000 + SelfInstanceId();
	for (int idx = 0; idx < transfer_config.Data().transfer_list_size(); ++idx) {
		const config::TransferInfo& item = transfer_config.Data().transfer_list(idx);
		// 自己不连自己
		if (item.svr_zone() == SelfServerZone()
			&& item.inst_id() == SelfInstanceId()) {
			continue;
		}
		// 只连zone+instid值比自己小的
		bool active = false;
		int seq = item.svr_zone() * 1000000 + item.inst_id();
		if (self_seq < seq) {
			if (!ExistTransfer(item.listen_ip(), item.listen_port2(), item.inst_id())) {
				active = true;
				NetIoHandlerSgl.ConnectOne(item.listen_ip(), item.listen_port2(),
					Enum_ConnType_Transfer2, item.inst_id());
			}
		}

		TransferSvrInfo info;
		info.fd = 0;
		info.ip = item.listen_ip();
		info.port = item.listen_port2();
		info.zone = item.svr_zone();
		info.inst_id = item.inst_id();
		info.online = false;
		info.active = active;
		tmp_zone_transfer_map[item.svr_zone()][item.inst_id()] = info;
	}

	// 关闭需要被关闭的
	for (auto iter1 = _zone_transfer_map.begin(); iter1 != _zone_transfer_map.end(); ++iter1) {
		for (auto iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2) {
			bool exist = false;
			for (int idx = 0; idx < transfer_config.Data().transfer_list_size(); ++idx) {
				auto& item = transfer_config.Data().transfer_list(idx);
				if (iter2->ip == item.listen_ip()
					&& iter2->port == item.listen_port2()
					&& iter2->inst_id == item.inst_id()) {
					exist = true;
					break;
				}
			}

			if (exist) {
				tmp_zone_transfer_map[iter2->zone][iter2->inst_id] = *iter2;
			}
			else {
				NetIoHandlerSgl.CloseFd(iter2->fd);
			}
		}
	}

	_zone_transfer_map.clear();
	for (auto iter1 = tmp_zone_transfer_map.begin(); iter1 != tmp_zone_transfer_map.end(); ++iter1) {
		for (auto iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2) {
			_zone_transfer_map[iter2->second.zone].push_back(iter2->second);
		}
	}
	return 0;
}