#include "transfersvr/transfer_inst_mgr.h"

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

	std::set<int> number_set;
	for (int idx = 0; idx < transfer_config.Data().transfer_list_size(); ++idx) {
		int number = transfer_config.Data().transfer_list(idx).number();
		if (number_set.count(number) > 0) {
			LogError("router number is duplicated: " << number);
			return -1;
		}
		number_set.insert(number);
	}

	if (0 != ConnectTransfers(transfer_config)) {
		LogError("ConnectTransfers fail");
		return -1;
	}

	return 0;
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

int TransferInstanceMgr::ConnectTransfers(ServerCfg<config::TransferConfig>& transfer_config) {
	// 设计原则是，多连少关
	std::unordered_map <base::s_int32_t, shared_ptr_t<TransferSvrInfo>> tmp_all_transfers;
	for (int idx = 0; idx < transfer_config.Data().transfer_list_size(); ++idx) {
		const config::TransferInfo& item = transfer_config.Data().transfer_list(idx);
		// 自己不连自己
		if (item.svr_zone() == SelfServerZone()
			&& item.inst_id() == SelfInstanceId()) {
			continue;
		}


	}

	return 0;
}