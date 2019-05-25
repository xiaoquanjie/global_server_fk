#include "transfersvr/transfersvr.h"
#include "slience/base/logger.hpp"
#include "commonlib/transaction/transaction_mgr.h"
#include "protolib/src/cmd.pb.h"
#include "commonlib/net_handler/net_handler.h"

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
	int port = CalcPort(0);
	if (!NetIoHandlerSgl.ListenOne(ip, port, Enum_ListenType_Transfer)) {
		LogError(ip << " " << port << "listen error:" << NetIoHandlerSgl.GetLastError().What());
		return -1;
	}
	else {
		LogInfo("listen in: " << ip << " " << port);
	}
	return 0;
}

int TransferApplication::OnReload() {
	const std::string config_path = ConfigFilePath();
	if (_svr_config.Parse(config_path.c_str()) != 0) {
		LogError("_svr_config.Parse fail");
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
	return 0;
}

int TransferApplication::OnProc(base::s_int64_t fd, const AppHeadFrame& frame, const char* data, base::s_uint32_t data_len) {
	return 0;
}

int TransferApplication::ForwardPkg(unsigned int dst_svr_type, int dst_inst_id, const AppHeadFrame& frame,
	const char* data, base::s_uint32_t data_len) {
	return 0;
}