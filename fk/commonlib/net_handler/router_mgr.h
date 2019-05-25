#pragma once
#include "slience/base/singletion.hpp"
#include "commonlib/svr_base/svrbase.h"
#include "protolib/src/router.pb.h"
#include "commonlib/svr_base/server_cfg.h"
#include <vector>

struct RouterInfo {
	std::string ip;
	unsigned short port;
	int number;
	base::s_int64_t fd;
};

class RouterMgr {
public:
	RouterMgr();

	int Init(int self_svr_type, 
		int self_inst_id, 
		int self_server_zone,
		const std::string& router_file);

	int Reload();

	void Tick(const base::timestamp& now);

	bool ExistRouter(const std::string& ip,
		unsigned int port,
		int number);

	int AddRouter(const std::string& ip,
		unsigned int port,
		int number,
		base::s_int64_t fd);

	int SendMsg(int cmd, 
		base::s_int64_t userid,
		bool is_broadcast,
		base::s_uint16_t dst_zone,
		base::s_uint32_t dst_svr_type, 
		base::s_uint32_t dst_inst_id,
		base::s_uint32_t src_trans_id, 
		base::s_uint32_t dst_trans_id,
		base::s_uint32_t req_random,
		google::protobuf::Message& msg);

	int SendMsgByFd(base::s_int64_t fd, 
		int cmd,
		base::s_int64_t userid, 
		bool is_broadcast,
		base::s_uint16_t dst_zone, 
		base::s_uint32_t dst_svr_type,
		base::s_uint32_t dst_inst_id,
		base::s_uint32_t src_trans_id, 
		base::s_uint32_t dst_trans_id,
		base::s_uint32_t req_random,
		google::protobuf::Message& msg);

	int SelfSeverType();

	int SelfInstanceId();

	int SelfServerZone();

protected:
	int ConnectRouters(ServerCfg<config::RouterConfig>& router_config);

private:
	std::string _router_file;
	std::vector<RouterInfo> _router_info_vec;
	base::timestamp _last_snd_heatbeat_time;

	int _self_server_type;
	int _self_instance_id;
	int _self_server_zone;
};

#ifndef RouterMgrSgl
#define RouterMgrSgl base::singleton<RouterMgr>::mutable_instance()
#endif
