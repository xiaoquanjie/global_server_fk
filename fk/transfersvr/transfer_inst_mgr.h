#pragma once

#include "slience/base/singletion.hpp"
#include "commonlib/svr_base/svrbase.h"
#include "protolib/src/transfer.pb.h"
#include "commonlib/svr_base/server_cfg.h"

struct TransferSvrInfo {
	std::string ip;
	unsigned int port;
	base::s_int64_t fd;
	base::s_int32_t zone;
	base::s_int32_t member;
	bool online;
};

class TransferInstanceMgr {
public:
	TransferInstanceMgr();

	int Init(int self_svr_type,
		int self_inst_id,
		int self_server_zone,
		const std::string& transfer_file);

	int Reload();  

	int SelfSeverType();

	int SelfInstanceId();

	int SelfServerZone();

protected:
	int ConnectTransfers(ServerCfg<config::TransferConfig>&);

private:
	std::string _transfer_file;
	int _self_server_type;
	int _self_instance_id;
	int _self_server_zone;

	// 所有的transfer(除了自己)，按member排好序
	std::unordered_map <base::s_int32_t, shared_ptr_t<TransferSvrInfo>> _all_transfers;
};  