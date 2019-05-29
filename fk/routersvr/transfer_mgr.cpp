#include "routersvr/transfer_mgr.h"
#include "slience/base/logger.hpp"
#include "commonlib/net_handler/net_handler.h"
#include "protolib/src/svr_base.pb.h"
#include "protolib/src/cmd.pb.h"

TransferMgr::TransferMgr() {
	_self_server_type = 0;
	_self_instance_id = 0;
	_self_server_zone = 0;
}

int TransferMgr::Init(int self_svr_type,
	int self_inst_id,
	int self_server_zone,
	const std::string& transfer_file) {
	_self_server_type = self_svr_type;
	_self_instance_id = self_inst_id;
	_self_server_zone = self_server_zone;
	_transfer_file = transfer_file;

	int ret = Reload();
	if (ret != 0) {
		LogError("reload fail");
	}
	return ret;
}

int TransferMgr::Reload() {
	if (_transfer_file.empty()) {
		return 0;
	}

	ServerCfg<config::TransferConfig> transfer_config;
	if (0 != transfer_config.Parse(_transfer_file.c_str())) {
		LogError("transfer_config.Parse fail: " << _transfer_file);
		return -1;
	}

	std::set<int> number_set;
	for (int idx = 0; idx < transfer_config.Data().transfer_list_size(); ++idx) {
		int number = transfer_config.Data().transfer_list(idx).number();
		if (number_set.count(number) > 0) {
			LogError("router number is duplicated: " << number);
			return -1;
		}
	}

	if (0 != ConnectTransfers(transfer_config)) {
		LogError("ConnectTransfers fail");
		return -1;
	}

	return 0;
}

void TransferMgr::Tick(const base::timestamp& now) {
	if ((now.second() - _last_snd_time.second()) < 20) {
		return;
	}

	_last_snd_time = now;
	proto::SvrHeatBeat msg;
	msg.set_server_type(SelfSeverType());
	msg.set_instance_id(SelfInstanceId());
	for (auto& info : _transfer_info_vec) {
		SendMsgByFd(info.fd,
			proto::CMD::CMD_SVR_HEATBEAT,
			0,
			SelfServerZone(),
			proto::SVR_TYPE_TRANSFER,
			info.number,
			0,
			0,
			msg);
	}
}

bool TransferMgr::ExistTransfer(const std::string& ip,
	unsigned int port,
	int number) {
	for (auto iter = _transfer_info_vec.begin(); iter != _transfer_info_vec.end(); ++iter) {
		if (ip == iter->ip
			&& port == iter->port
			&& number == iter->number) {
			return true;
		}
	}
	return false;
}

int TransferMgr::AddTransfer(const std::string& ip,
	unsigned int port,
	int number,
	base::s_int64_t fd) {
	for (auto iter = _transfer_info_vec.begin(); iter != _transfer_info_vec.end();
		++iter) {
		if (ip == iter->ip
			&& port == iter->port
			&& number == iter->number) {
			iter->fd = fd;
			return 0;
		}
	}
	return -1;
}

int TransferMgr::GetTransferInstId(base::s_uint64_t userid, std::vector<base::s_uint64_t>& inst_vec) {
	if (_transfer_info_vec.empty()) {
		return -1;
	}

	int r = userid % _transfer_info_vec.size();
	inst_vec.push_back(_transfer_info_vec[r].fd);
	return 0;
}

int TransferMgr::SendMsgByFd(base::s_uint64_t fd,
	int cmd,
	base::s_uint64_t userid,
	base::s_uint16_t dst_zone,
	base::s_uint32_t dst_svr_type,
	base::s_uint32_t dst_inst_id,
	base::s_uint32_t src_trans_id,
	base::s_uint32_t dst_trans_id,
	google::protobuf::Message& msg) {
	AppHeadFrame frame;
	frame.set_is_broadcast(false);
	frame.set_src_zone(SelfServerZone());
	frame.set_dst_zone(dst_zone);
	frame.set_src_svr_type(SelfSeverType());
	frame.set_dst_svr_type(dst_svr_type);
	frame.set_src_inst_id(SelfInstanceId());
	frame.set_dst_inst_id(dst_trans_id);
	frame.set_src_trans_id(src_trans_id);
	frame.set_dst_trans_id(dst_trans_id);
	frame.set_cmd(cmd);
	frame.set_userid(userid);
	frame.set_req_random(0);

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
	return 0;
}

int TransferMgr::ConnectTransfers(ServerCfg<config::TransferConfig>& transfer_config) {
	// 设计原则是，多连少关
	std::map<int, TransferInfo> transfer_info_map;
	for (int idx = 0; idx < transfer_config.Data().transfer_list_size(); ++idx) {
		auto& item = transfer_config.Data().transfer_list(idx);
		// 只连接相同区的transfer
		if (item.svr_zone() != SelfServerZone()) {
			continue;
		}
		if (!ExistTransfer(item.listen_ip(), item.listen_port(), item.number())) {
			NetIoHandlerSgl.ConnectOne(item.listen_ip(), item.listen_port(),
				Enum_ConnType_Transfer, item.number());

			TransferInfo transfer_info;
			transfer_info.ip = item.listen_ip();
			transfer_info.port = item.listen_port();
			transfer_info.number = item.number();
			transfer_info.fd = 0;
			transfer_info_map[item.number()] = transfer_info;
		}
	}

	// 需要被关闭了的
	for (auto iter = _transfer_info_vec.begin(); iter != _transfer_info_vec.end();) {
		bool exist = false;
		for (int idx = 0; idx < transfer_config.Data().transfer_list_size(); ++idx) {
			auto& item = transfer_config.Data().transfer_list(idx);
			if (iter->ip == item.listen_ip()
				&& iter->port == item.listen_port()
				&& iter->number == item.number()) {
				exist = true;
				break;
			}
		}
		if (!exist) {
			NetIoHandlerSgl.CloseFd(iter->fd);
		}
		else {
			transfer_info_map[iter->number] = *iter;
			iter++;
		}
	}

	_transfer_info_vec.clear();
	for (auto iter = transfer_info_map.begin(); iter != transfer_info_map.end();
		++iter) {
		_transfer_info_vec.push_back(iter->second);
	}
	return 0;
}

int TransferMgr::SelfSeverType() {
	return _self_server_type;
}

int TransferMgr::SelfInstanceId() {
	return _self_instance_id;
}

int TransferMgr::SelfServerZone() {
	return _self_server_zone;
}