#include "routersvr/router_svr.h"
#include "slience/base/logger.hpp"
#include "commonlib/transaction/transaction_mgr.h"
#include "protolib/src/cmd.pb.h"
#include "routersvr/server_instance_mgr.h"
#include "commonlib/net_handler/net_handler.h"
#include "routersvr/transfer_mgr.h"

int RouterApplication::ServerType() {
	return proto::SVR_TYPE_ROUTER;
}

int RouterApplication::InstanceId() {
	return _svr_config.Data().svr_inst_id();
}

int RouterApplication::ServerZone() {
	return _svr_config.Data().svr_zone();
}

int RouterApplication::OnInitNetWork() {
	// start network thread
	auto func = m_bind_t(&RouterApplication::OnProc, this, placeholder_1,
		placeholder_2, placeholder_3, placeholder_4);
	NetIoHandlerSgl.Init(_now, func);
	NetIoHandlerSgl.Start(_svr_thread_cnt, false);
	return 0;
}

void RouterApplication::OnStopNetWork() {
	NetIoHandlerSgl.Stop();
}

int RouterApplication::UpdateNetWork() {
	return NetIoHandlerSgl.Update();
}

int RouterApplication::OnInit() {
	// listen
	std::string ip = _svr_config.Data().listen_ip();
	int port = CalcPort(0);
	if (!NetIoHandlerSgl.ListenOne(ip, port, Enum_ListenType_Router)) {
		LogError(ip << " " << port << "listen error:" << NetIoHandlerSgl.GetLastError().What());
		return -1;
	}
	else {
		LogInfo("listen in: " << ip << " " << port);
	}
	LogInfo("server listen success");

	std::string transfer_file = _confdir + _comm_config.Data().transfer_conf_file();
	int ret = TransferMgrSgl.Init(ServerType(), InstanceId(), ServerZone(), transfer_file);
	if (0 != ret) {
		return -1;
	}

	SeverInstanceMgrSgl.Init(&_svr_config.Data());
	return 0;
}

int RouterApplication::OnReload() {
	const std::string config_path = ConfigFilePath();
	if (_svr_config.Parse(config_path.c_str()) != 0) {
		LogError("_svr_config.Parse fail");
		return -1;
	}
	if (0 != TransferMgrSgl.Reload()) {
		LogError("TransferMgrSgl.Reload fail");
		return -1;
	}
	return 0;
}

int RouterApplication::OnExit() {
	OnStopNetWork();
	return 0;
}

int RouterApplication::OnTick(const base::timestamp& now) {
	NetIoHandlerSgl.OnTick();
	if (TickCount() % (100 * 20) == 0) {
		TransferMgrSgl.Tick(now);
	}
	return 0;
}

int RouterApplication::OnProc(base::s_int64_t fd, const AppHeadFrame& frame, const char* data, base::s_uint32_t data_len) {
	switch (frame.get_cmd()) {
	case proto::CMD::CMD_SOCKET_CLIENT_OUT:
	case proto::CMD::CMD_SOCKET_CLIENT_IN:
	case proto::CMD::CMD_REGISTER_SERVER_REQ:
	case proto::CMD::CMD_REGISTER_SERVER_RSP:
	case proto::CMD::CMD_SVR_HEATBEAT:
		TransactionMgr::ProcessFrame(fd, ServerType(), InstanceId(), ServerZone(), frame, data);
		return 0;
	default:
		// 转发
		break;
	}
	if (frame.get_dst_zone() == ServerZone()) {
		// 同一个服务区
		if (frame.get_dst_inst_id() != 0) {
			ForwardPkg(frame.get_dst_svr_type(), frame.get_dst_inst_id(), frame, data, data_len);
		}
		else {
			std::vector<int> inst_vec;
			SeverInstanceMgrSgl.RouterPolicy(frame.get_userid(), frame.get_dst_svr_type(),
				frame.get_is_broadcast(), inst_vec);
			for (auto iter = inst_vec.begin(); iter != inst_vec.end(); ++iter) {
				ForwardPkg(frame.get_dst_svr_type(), *iter, frame, data, data_len);
			}
		}
	}
	else {
		// 不是同一个服务,转发到transfersvr服
		ForwarPkgToTransfer(frame, data, data_len);
	}
	return 0;
}

int RouterApplication::ForwardPkg(unsigned int dst_svr_type, int dst_inst_id, const AppHeadFrame& frame, const char* data, base::s_uint32_t data_len) {
	const char* src = data - sizeof(frame);
	int src_len = data_len + sizeof(frame);
	base::s_int64_t fd = SeverInstanceMgrSgl.GetFdByTypeId(dst_svr_type, dst_inst_id);
	if (fd != 0) {
		NetIoHandlerSgl.SendDataByFd(fd, src, src_len);
	}
	return 0;
}

int RouterApplication::ForwarPkgToTransfer(const AppHeadFrame& frame, const char* data, base::s_uint32_t data_len) {
	std::vector<base::s_uint64_t> inst_vec;
	TransferMgrSgl.GetTransferInstId(frame.get_userid(), inst_vec);
	if (inst_vec.empty()) {
		LogError("no transfersvr to send");
		return -1;
	}

	const char* src = data - sizeof(frame);
	int src_len = data_len + sizeof(frame);
	for (auto fd : inst_vec) {
		NetIoHandlerSgl.SendDataByFd(fd, src, src_len);
	}
	return 0;
}