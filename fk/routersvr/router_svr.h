#pragma once

#include "commonlib/svr_base/ApplicationBase.h"
#include "slience/base/singletion.hpp"
#include "protolib/src/routersvr_config.pb.h"
#include "commonlib/svr_base/server_cfg.h"

class RouterApplication : public ApplicationBase {
protected:
	int ServerType() override;

	int InstanceId() override;

	int ServerZone() override;

	int OnInitNetWork() override;

	void OnStopNetWork() override;

	int UpdateNetWork() override;

	int OnInit() override;

	int OnReload() override;

	int OnExit() override;

	int OnTick(const base::timestamp& now) override;

	int OnProc(base::s_int64_t fd, const AppHeadFrame& frame, const char* data, base::s_uint32_t data_len) override;

	int ForwardPkg(unsigned int dst_svr_type, int dst_inst_id, const AppHeadFrame& frame, const char* data, base::s_uint32_t data_len);

private:
	ServerCfg<config::RouterSvrConfig> _svr_config;
};

#ifndef RouterAppSgl
#define RouterAppSgl base::singleton<RouterApplication>::mutable_instance()
#endif
