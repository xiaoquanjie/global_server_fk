#include "connsvr/conn_svr.h"
#include "commonlib/net_handler/net_handler.h"
#include "commonlib/transaction/transaction_mgr.h"
#include "commonlib/net_handler/router_mgr.h"

int ConnApplication::ServerType() {
	return proto::SVR_TYPE_CONN;
}

int ConnApplication::InstanceId() {
	return _svr_config.Data().svr_inst_id();
}

int ConnApplication::ServerZone() {
	return _svr_config.Data().svr_zone();
}

int ConnApplication::OnInitNetWork() {
	auto func = m_bind_t(&ConnApplication::OnProc, this, placeholder_1,
		placeholder_2, placeholder_3, placeholder_4);
	NetIoHandlerSgl.Init(_now, func);
	NetIoHandlerSgl.Start(_svr_thread_cnt, false);
	return 0;
}

void ConnApplication::OnStopNetWork() {
	NetIoHandlerSgl.Stop();
}

int ConnApplication::UpdateNetWork() {
	return NetIoHandlerSgl.Update();
}

int ConnApplication::OnInit() {
	// listen
	std::string ip = _svr_config.Data().listen_ip();
	int port = CalcPort(0);
	if (!NetIoHandlerSgl.ListenOne(ip, port, Enum_ListenType_Conn)) {
		LogError(ip << " " << port << "listen error:" << NetIoHandlerSgl.GetLastError().What());
		return -1;
	}
	else {
		LogInfo("listen in: " << ip << " " << port);
	}

	std::string router_conf_file = _confdir + _comm_config.Data().router_conf_file();
	int ret = RouterMgrSgl.Init(ServerType(),
		InstanceId(), 
		ServerZone(),
		router_conf_file);
	if (0 != ret) {
		return -1;
	}

	return 0;
}

int ConnApplication::OnReload() {
	const std::string path = ConfigFilePath();
	if (0 != _svr_config.Parse(path.c_str())) {
		LogError("_svr_config.Parse fail:" << path);
		return -1;
	}

	if (0 != RouterMgrSgl.Reload()) {
		LogError("RouterMgrSgl.Reload fail");
		return -1;
	}
	return 0;
}

int ConnApplication::OnExit() {
	OnStopNetWork();
	return 0;
}

int ConnApplication::OnTick(const base::timestamp& now) {
	NetIoHandlerSgl.OnTick();
	if (TickCount() % (100 * 20) == 0) {
		RouterMgrSgl.Tick(now);
	}
	return 0;
}

int ConnApplication::OnProc(base::s_int64_t fd, const AppHeadFrame& frame, const char* data, base::s_uint32_t data_len) {
	TransactionMgr::ProcessFrame(fd, ServerType(), InstanceId(), ServerZone(), frame, data);
	return 0;
}