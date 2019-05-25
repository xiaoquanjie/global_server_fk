#pragma once

#include "slience/base/singletion.hpp"
#include "commonlib/svr_base/svrbase.h"
#include "protolib/src/transfer.pb.h"
#include "commonlib/svr_base/server_cfg.h"

struct TransferInfo {
	std::string ip;
	unsigned short port;
	int number;
	base::s_uint64_t fd;
};

class TransferMgr {
public:
	TransferMgr();

	int Init(int self_svr_type, 
		int self_inst_id,
		int self_server_zone,
		const std::string& transfer_file);

	int Reload();

	bool ExistTransfer(const std::string& ip,
		unsigned int port,
		int number);

	int AddTransfer(const std::string& ip,
		unsigned int port, 
		int number,
		base::s_int64_t fd);

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

	std::vector<TransferInfo> _transfer_info_vec;
};

#ifndef TransferMgrSgl
#define TransferMgrSgl base::singleton<TransferMgr>::mutable_instance()
#endif