#include "server_agent/server_agent.h"
#include "commonlib/net_handler/net_handler.h"
#include <climits>
#include "slience/base/util.h"
#include <fstream>

const std::string agent_data_file = "./agent_data_file";

int AgentApplication::ServerType() {
	return -1;
}

int AgentApplication::InstanceId() {
	return -1;
}

int AgentApplication::OnInitNetWork() {
	auto func = m_bind_t(&AgentApplication::OnProc, this, placeholder_1,
		placeholder_2, placeholder_3, placeholder_4);
	NetIoHandlerSgl.Init(_now, func);
	NetIoHandlerSgl.Start(_svr_thread_cnt, false);
	return 0;
}

void AgentApplication::OnStopNetWork() {
	NetIoHandlerSgl.Stop();
}

int AgentApplication::UpdateNetWork() {
	NetIoHandlerSgl.Update();
	return 0;
}

int AgentApplication::OnInit() {
	// listen
	std::string ip = _svr_config.Data().listen_info().listen_ip();
	int port = _svr_config.Data().listen_info().listen_port();
	if (!NetIoHandlerSgl.ListenOne(ip, port, 0)) {
		LogError(ip << " " << port << "listen error:" << NetIoHandlerSgl.GetLastError().What());
		return -1;
	}
	else {
		LogInfo("listen in: " << ip << " " << port);
	}

	LoadAgentDataFile();
	return 0;
}

int AgentApplication::OnReload() {
	const std::string path = ConfigFilePath();
	if (0 != _svr_config.Parse(path.c_str())) {
		LogError("_svr_config.Parse fail:" << path);
		return -1;
	}

	return 0;
}

int AgentApplication::OnExit() {
	OnStopNetWork();
	return 0;
}

int AgentApplication::OnTick(const base::timestamp& now) {
	NetIoHandlerSgl.OnTick();
	ExecuteScript();

	static time_t next_check_alive = now.second();
	if (now.second() >= next_check_alive) {
		CheckAlive();
		next_check_alive = now.second() + 10;
	}
	return 0;
}

int AgentApplication::OnProc(base::s_int64_t fd, const AppHeadFrame& frame, const char* data, base::s_uint32_t data_len) {
	// TransactionMgr::ProcessFrame(fd, ServerType(), InstanceId(), frame, data);
	if (frame.get_cmd() == agent::CMD_EXECUTE_OMMAND_REQ) {
		agent::ExecuteCommandReq request;
		agent::ExecuteCommandRsp respond;
	
		if (!request.ParseFromArray(data, data_len)) {
			LogError("parse protobuf fail, cmd:" << frame.get_cmd());
			return -1;
		}
			
		LogInfo("request: " << request.ShortDebugString());
		if (request.cur_cmd() == request.instance().start_cmd()) {
			OnStartCmd(request, respond);
		}
		else if (request.cur_cmd() == request.instance().stop_cmd()) {
			OnStopCmd(request, respond);
		}
		else if (request.cur_cmd() == request.instance().clean_cmd()) {
			OnCleanCmd(request, respond);
		}
		else if (request.cur_cmd() == request.instance().check_cmd()) {
			OnCheckCmd(request, respond);
		}
		else {
			OnCmd(request, respond);
		}

		LogInfo("respond: " << respond.ShortDebugString());
		AppHeadFrame respond_frame;
		respond_frame.set_cmd(agent::CMD_EXECUTE_OMMAND_RSP);
		std::string respond_data = respond.SerializePartialAsString();
		respond_frame.set_cmd_length(respond_data.length());

		base::Buffer buffer;
		buffer.Write(respond_frame);
		buffer.Write(respond_data.c_str(), respond_data.length());
		NetIoHandlerSgl.SendDataByFd(fd, buffer.Data(), buffer.Length());
	}
	else {
		LogError("illegal cmd:" << frame.get_cmd());
		return -1;
	}
	return 0;
}

void AgentApplication::ExecuteScript() {
	const base::timestamp& now = GetNow();
	for (int idx = 0; idx < _svr_config.Data().script_info_list_size(); ++idx) {
		auto& item = *(_svr_config.Data().mutable_script_info_list(idx));
		time_t next_time = 0;
		if (item.type() == config::E_SCRIPT_TYPE_CERTAIN_TIME) {
			next_time = base::CertainTimesOfDay(now.second(), item.day_certain_time());
			if (now.second() >= next_time) {
				next_time += 24 * 3600;
			}
		}
		else if (item.type() == config::E_SCRIPT_TYPE_PERIOD_TIME) {
			next_time = base::ClosestTimeOfDay(now.second(), item.interval_seconds());
		}

		if (!item.has_next_check_time()) {
			item.set_next_check_time((unsigned int)next_time);
			LogInfo(item.script() << " script next execute time is: " << next_time);
		}

		if (now.second() < item.next_check_time()) {
			continue;
		}

#ifndef M_PLATFORM_WIN
		char cwd[PATH_MAX];
		getcwd(cwd, sizeof(cwd));
		chdir(item.script_dir().c_str());
		System(item.script());
		chdir(cwd);
#endif
		item.set_next_check_time((unsigned int)next_time);
		LogInfo(item.script() << " script next execute time is: " << next_time);
	}
}

int AgentApplication::System(const std::string& cmd) {
#ifndef M_PLATFORM_WIN
	signal(SIGCHLD, SIG_DFL);
	int ret = system(cmd.c_str());
	signal(SIGCHLD, SIG_IGN);
	if (ret < 0) {
		LogError("fail to call system " << cmd << ", return: " << ret << ", errno: " << errno);
	}
	return ret;
#else
	return 0;
#endif
}

void AgentApplication::CheckAlive() {
	std::vector<std::string> del_instances;
	for (auto item : _agent_data.instance_list()) {
		if (item.check_cmd().empty()) {
			continue;
		}

		int ret = System(item.check_cmd());
		if (ret == 0) {
			item.set_check_cnt(0);
			continue;
		}

		if ((unsigned int)item.check_cnt() > _svr_config.Data().max_pullup_server_cnt()) {
			del_instances.push_back(item.inst_name());
			LogError(item.inst_name() << " server been pull up too much times");
			continue;
		}

		ret = System(item.start_cmd());
		item.set_check_cnt(item.check_cnt() + 1);
		LogInfo(item.inst_name() << " server is stop, try to pull up it, ret:" << ret);
	}

	for (const auto &item : del_instances) {
		DelInstance(item);
	}
}

int AgentApplication::LoadAgentDataFile() {
	std::ifstream ifs(agent_data_file.c_str(), std::ios::in);
	if (!ifs) {
		LogError("open file failed, file_path=" << agent_data_file);
		return -1;
	}

	google::protobuf::io::IstreamInputStream inputStream(&ifs);
	if (!google::protobuf::TextFormat::Parse(&inputStream, &_agent_data)) {
		LogError("google::protobuf::TextFormat::Parse failed, file_path=" << agent_data_file);
		ifs.close();
		return -2;
	}
	ifs.close();
	LogInfo("parse server cfg file success, file_path=" << agent_data_file);
	return 0;
}

void AgentApplication::SaveAgentDataFile() {
	std::ofstream ofs(agent_data_file.c_str(), std::ios::out | std::ios::trunc);
	if (!ofs) {
		LogError("fail to open file: " << agent_data_file);
		return;
	}

	ofs << _agent_data.DebugString();
	ofs.close();
}

const agent::Instance* AgentApplication::GetInstance(const std::string& name) {
	for (auto& item : _agent_data.instance_list()) {
		if (item.inst_name() == name) {
			return &item;
		}
	}
	return 0;
}

void AgentApplication::AddInstance(const agent::Instance& instance) {
	auto item = GetInstance(instance.inst_name());
	if (!item) {
		return;
	}

	LogInfo("add instance: " << instance.ShortDebugString());
	auto inst = _agent_data.add_instance_list();
	inst->CopyFrom(instance);
	SaveAgentDataFile();
	return;
}

void AgentApplication::DelInstance(const std::string& name) {
	for (int idx = 0; idx < _agent_data.instance_list_size(); ++idx) {
		auto item = _agent_data.mutable_instance_list(idx);
		if (item->inst_name() == name) {
			LogInfo("del instance: " << item->ShortDebugString());
			_agent_data.mutable_instance_list()->DeleteSubrange(idx, 1);
			return;
		}
	}
}

int AgentApplication::OnStartCmd(agent::ExecuteCommandReq& request, agent::ExecuteCommandRsp& respond) {
	if (0 == OnCmd(request, respond)) {
		AddInstance(request.instance());
		return 0;
	} 
	return -1; 
}

int AgentApplication::OnStopCmd(agent::ExecuteCommandReq& request, agent::ExecuteCommandRsp& respond) {
	int ret = OnCmd(request, respond);
	DelInstance(request.instance().inst_name());
	return ret;
}

int AgentApplication::OnCleanCmd(agent::ExecuteCommandReq& request, agent::ExecuteCommandRsp& respond) {
	return OnCmd(request, respond);
}

int AgentApplication::OnCheckCmd(agent::ExecuteCommandReq& request, agent::ExecuteCommandRsp& respond) {
	return OnCmd(request, respond);
}

int AgentApplication::OnCmd(agent::ExecuteCommandReq& request, agent::ExecuteCommandRsp& respond) {
	int ret = System(request.cur_cmd());
	if (ret != 0) {
		LogError("fail to call cmd: " << request.cur_cmd());
		respond.set_ret(-1);
		return -1;
	}
	return 0;
}

