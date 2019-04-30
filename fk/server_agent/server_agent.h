#pragma once

#include "slience/base/singletion.hpp"
#include "protolib/src/server_agent_conf.pb.h"
#include "protolib/src/server_agent_data.pb.h"
#include "commonlib/svr_base/server_cfg.h"
#include "commonlib/svr_base/ApplicationBase.h"

class AgentApplication : public ApplicationBase {
public:
	int ServerType() override;

	int InstanceId() override;

	int OnInitNetWork() override;

	void OnStopNetWork() override;

	int UpdateNetWork() override;

	int OnInit() override;

	int OnReload() override;

	int OnExit() override;

	int OnTick(const base::timestamp& now) override;

	int OnProc(base::s_int64_t fd, const AppHeadFrame& frame, const char* data, base::s_uint32_t data_len) override;

protected:
	void ExecuteScript();

	int System(const std::string& cmd);

	void CheckAlive();

	int LoadAgentDataFile();

	void SaveAgentDataFile();

	const agent::Instance* GetInstance(const std::string& name);

	void AddInstance(const agent::Instance& instance);

	void DelInstance(const std::string& name);

	int OnStartCmd(agent::ExecuteCommandReq& request, agent::ExecuteCommandRsp& respond);

	int OnStopCmd(agent::ExecuteCommandReq& request, agent::ExecuteCommandRsp& respond);

	int OnCleanCmd(agent::ExecuteCommandReq& request, agent::ExecuteCommandRsp& respond);

	int OnCheckCmd(agent::ExecuteCommandReq& request, agent::ExecuteCommandRsp& respond);

	int OnCmd(agent::ExecuteCommandReq& request, agent::ExecuteCommandRsp& respond);

private:
	ServerCfg<config::ServerAgentConfig> _svr_config;
	agent::SvrAgentData _agent_data;
};

#ifndef AgentApplicationSgl
#define AgentApplicationSgl base::singleton<AgentApplication>::mutable_instance()
#endif