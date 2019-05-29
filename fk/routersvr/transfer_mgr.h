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

	void Tick(const base::timestamp& now);

	bool ExistTransfer(const std::string& ip,
		unsigned int port,
		int number);

	int AddTransfer(const std::string& ip,
		unsigned int port, 
		int number,
		base::s_int64_t fd);

	int GetTransferInstId(base::s_uint64_t userid, std::vector<base::s_uint64_t>& inst_vec);

	int SelfSeverType();

	int SelfInstanceId();

	int SelfServerZone();

protected:
	int ConnectTransfers(ServerCfg<config::TransferConfig>&);

	int SendMsgByFd(base::s_uint64_t fd,
		int cmd,
		base::s_uint64_t userid,
		base::s_uint16_t dst_zone,
		base::s_uint32_t dst_svr_type,
		base::s_uint32_t dst_inst_id,
		base::s_uint32_t src_trans_id,
		base::s_uint32_t dst_trans_id,
		google::protobuf::Message& msg);

private:
	std::string _transfer_file;
	int _self_server_type;
	int _self_instance_id;
	int _self_server_zone;

	base::timestamp _last_snd_time;
	std::vector<TransferInfo> _transfer_info_vec;
};

#ifndef TransferMgrSgl
#define TransferMgrSgl base::singleton<TransferMgr>::mutable_instance()
#endif