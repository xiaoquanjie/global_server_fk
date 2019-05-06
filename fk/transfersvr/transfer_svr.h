#pragma once

#include "commonlib/svr_base/ApplicationBase.h"
#include "slience/base/singletion.hpp"
#include "protolib/src/transfersvr.pb.h"
#include "commonlib/svr_base/server_cfg.h"

class TransferApplication : public ApplicationBase {
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

private:
	ServerCfg<config::TransferSvrConfig> _svr_config;
};

#ifndef TransferAppSgl
#define TransferAppSgl base::singleton<TransferApplication>::mutable_instance()
#endif
