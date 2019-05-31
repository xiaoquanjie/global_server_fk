#include "transfersvr/transfersvr.h"
#include "slience/base/logger.hpp"
#include "commonlib/transaction/transaction_mgr.h"
#include "protolib/src/cmd.pb.h"
#include "commonlib/net_handler/net_handler.h"
#include "transfersvr/transfer_inst_mgr.h"
#include "transfersvr/router_inst_mgr.h"
#include "transfersvr/user_login_mgr.h"

int TransferApplication::ServerType() {
	return proto::SVR_TYPE_TRANSFER;
}

int TransferApplication::InstanceId() {
	return _svr_config.Data().svr_inst_id();
}

int TransferApplication::ServerZone() {
	return _svr_config.Data().svr_zone();
}

int TransferApplication::OnInitNetWork() {
	// start network thread
	auto func = m_bind_t(&TransferApplication::OnProc,
		this,
		placeholder_1,
		placeholder_2,
		placeholder_3,
		placeholder_4);
	NetIoHandlerSgl.Init(_now, func);
	NetIoHandlerSgl.Start(_svr_thread_cnt, false);
	return 0;
}

void TransferApplication::OnStopNetWork() {
	NetIoHandlerSgl.Stop();
}

int TransferApplication::UpdateNetWork() {
	return NetIoHandlerSgl.Update();
}

int TransferApplication::OnInit() {
	// listen
	std::string ip = _svr_config.Data().listen_ip();

	// 用于router连
	int port = CalcPort(Enum_ListenType_Transfer);
	if (!NetIoHandlerSgl.ListenOne(ip, port, Enum_ListenType_Transfer)) {
		LogError(ip << " " << port << "listen error:" << NetIoHandlerSgl.GetLastError().What());
		return -1;
	}
	else {
		LogInfo("listen in: " << ip << " " << port);
	}

	// 用于transfer之间连
	int port2 = CalcPort(Enum_ListenType_Transfer2);
	if (!NetIoHandlerSgl.ListenOne(ip, port2, Enum_ListenType_Transfer2)) {
		LogError(ip << " " << port2 << "listen error:" << NetIoHandlerSgl.GetLastError().What());
		return -1;
	}
	else {
		LogInfo("listen in: " << ip << " " << port2);
	}

	std::string transfer_file = _confdir + _comm_config.Data().transfer_conf_file();
	int ret = TransferInstanceMgrSgl.Init(ServerType(), InstanceId(), ServerZone(), transfer_file);
	if (0 != ret) {
		return -1;
	}
	return 0;
}

int TransferApplication::OnReload() {
	const std::string config_path = ConfigFilePath();
	if (_svr_config.Parse(config_path.c_str()) != 0) {
		LogError("_svr_config.Parse fail");
		return -1;
	}
	if (0 != TransferInstanceMgrSgl.Reload()) {
		LogError("TransferInstanceMgrSgl.Reload fail");
		return -1;
	}
	return 0;
}

int TransferApplication::OnExit() {
	OnStopNetWork();
	return 0;
}

int TransferApplication::OnTick(const base::timestamp& now) {
	NetIoHandlerSgl.OnTick();
	if (TickCount() % (100 * 20) == 0) {
		TransferInstanceMgrSgl.Tick(now);
	}
	if (TickCount() % (100 * 5) == 0) {
		UserLoginMgrSgl.Tick(now);
	}
	return 0;
}

int TransferApplication::OnProc(base::s_int64_t fd, const AppHeadFrame& frame, const char* data, base::s_uint32_t data_len) {
	switch (frame.get_cmd()) {
	case proto::CMD::CMD_SOCKET_CLIENT_OUT:
	case proto::CMD::CMD_SOCKET_CLIENT_IN:
	case proto::CMD::CMD_REGISTER_SERVER_REQ:
	case proto::CMD::CMD_REGISTER_SERVER_RSP:
	case proto::CMD::CMD_SVR_HEATBEAT:
	case proto::CMD::CMD_TRANSFER_HEATBEAT:
		TransactionMgr::ProcessFrame(fd, ServerType(), InstanceId(), ServerZone(), frame, data);
		return 0;
	default:
		// 转发
		break;
	}

	if (frame.get_dst_zone() != 0) {
		// 查找相应的zone区transfer服务
	}
	else {
		// 根据uid找出它所在的transfer服务区
	}
	return 0;
}

int TransferApplication::ForwardPkg(unsigned int dst_svr_type, int dst_inst_id, const AppHeadFrame& frame,
	const char* data, base::s_uint32_t data_len) {
	return 0;
}

base::s_uint32_t TransferApplication::CalcPort(int type) {
	// 一个server占用两个端口，一个server预留10个端口
	if (type == Enum_ListenType_Transfer) {
		return (PortStart() + InstanceId() * 10 + 1);
	}
	else {
		return (PortStart() + InstanceId() * 10 + 2);
	}
}