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
	base::s_int32_t inst_id;
	bool online;
	bool active;	// 标识是主动还是被动连接
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

	bool ExistTransfer(const std::string& ip,
		unsigned int port,
		base::s_int32_t inst_id);

	// 主动
	int LoginTransfer(const std::string& ip,
		unsigned int port,
		base::s_int32_t inst_id,
		base::s_int64_t fd);

	// 被动
	int LoginTransfer(base::s_int32_t server_zone, base::s_int32_t inst_id, base::s_int64_t fd);

	int LogoutTransfer(base::s_int32_t server_zone, base::s_int32_t inst_id, base::s_int64_t fd);

	int SendMsgToTransferByFd(base::s_uint64_t fd,
		int cmd,
		base::s_uint64_t userid,
		base::s_uint16_t dst_zone,
		base::s_uint32_t dst_svr_type,
		base::s_uint32_t dst_inst_id,
		base::s_uint32_t src_trans_id,
		base::s_uint32_t dst_trans_id,
		base::s_uint32_t req_random,
		google::protobuf::Message& msg);

protected:
	int ConnectTransfers(ServerCfg<config::TransferConfig>&);

private:
	std::string _transfer_file;
	int _self_server_type;
	int _self_instance_id;
	int _self_server_zone;

	// 所有的transfer(除了自己)
	std::unordered_map <base::s_int32_t, std::vector<TransferSvrInfo>> _zone_transfer_map;

	base::Buffer _buffer;
	AppHeadFrame _frame;
};  

#ifndef TransferInstanceMgrSgl
#define TransferInstanceMgrSgl base::singleton<TransferInstanceMgr>::mutable_instance()
#endif