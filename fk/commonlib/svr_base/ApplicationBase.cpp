#include "commonlib/svr_base/ApplicationBase.h"
#include "slience/base/string_util.hpp"
#include "slience/base/logger.hpp"
#include "commonlib/svr_base/getopt.hpp"
#include "commonlib/svr_base/ApplicationFunc.hpp"

#include "mysql_wrapper/mysql_wrapper.h"
#include "protolib/src/svr_base.pb.h"
#include "protolib/src/cmd.pb.h"
#include "commonlib/transaction/transaction_mgr.h"
#include "commonlib/async/async_mysql_mgr.h"
#include "commonlib/async/async_redis_mgr.h"
#include "commonlib/svr_base/svralloc.h"

/////////////////////////////////////////////////////////////

const char* App_Cmd_Stop = "stop";
const char* App_Cmd_Reload = "reload";

ApplicationBase::ApplicationBase() {
	_log_level = base::logger::LOG_LEVEL_TRACE;
	_log_withpid = 0;
	_daemon = 0;
	_svr_thread_cnt = 4;
	_total_tick_count_ = 0;
}

ApplicationBase::~ApplicationBase() {

}

int ApplicationBase::Init(int argc, char** argv) {
	int ret = 0;
	do {
		// set core file unlimit
		CoreFileUnlimit();

		ret = ParseOpt(argc, argv);
		if (0 != ret)
			break;

		// set work dir
		SetWorkDir(_workdir);

		// can't start two process
		ret = KillExist(_pid_file);
		if (ret != 0) {
			printf("can't kill old process\n");
			exit(0);
		}

		// set daemon
		if (_daemon == 1) {
			Daemon();
		}

		// init signal
		InitSigHandler();

		// init log
		SetLogFileName(_log_file, (bool)_log_withpid);
		SetLogLevel(_log_level);

		ret = WritePid(_pid_file);
		if (0 != ret) {
			break;
		}

		ret = _comm_config.Parse(_comm_conf_file.c_str());
		if (0 != ret) {
			LogError("_comm_config.Parse fail: " << _comm_conf_file);
			break;
		}

		// init network
		ret = OnInitNetWork();
		if (0 != ret) {
			LogError("OnInitNetWork error");
		}

		ret = OnReload();
		if (ret != 0) {
			LogError("reload error........");
			break;
		}

		ret = OnInit();
		if (ret != 0) {
			LogError("application init error.....");
			break;
		}

		// init transaction manager
		TransactionMgr::Init();

		if (UseAsyncMysql()) {
			ret = OnInitAsncMysql();
			if (ret != 0) {
				LogError("async mysql init error.....");
				break;
			}
		}

		if (UseAsyncRedis()) {
			ret = OnInitAsyncRedis();
			if (ret != 0) {
				LogError("async redis init error.....");
				break;
			}
		}

	} while (false);
	  
	if (0 == ret) {
		LogInfo("application param : workdir=" << GetWorkDir());
		LogInfo("application param : appname=" << _appname);
		LogInfo("application param : log_file=" << _log_file);
		LogInfo("application param : log_level=" << _log_level);
		LogInfo("application start successfully");
	}
	else {
		LogError("application start fail");
		StopLogger();
		// stop network 
		OnStopNetWork();
		exit(-1);
	}
	return ret;
}

int ApplicationBase::OnInit() {
	return 0;
}

int ApplicationBase::OnTick(const base::timestamp& now) {
	return 0;
}

int ApplicationBase::OnReload() {
	return 0;
}

int ApplicationBase::OnProc(base::s_int64_t fd, const AppHeadFrame& frame, const char* data, base::s_uint32_t data_len) {
	return -1;
}

int ApplicationBase::OnExit() {
	OnStopNetWork();
	return 0;
}

bool ApplicationBase::UseAsyncMysql() {
	return false;
}

int ApplicationBase::OnInitAsncMysql() {
	return AsyncMysqlMgr::Init(4, 40);
}

bool ApplicationBase::UseAsyncRedis() {
	return false;
}

int ApplicationBase::OnInitAsyncRedis() {
	return AsyncRedisMgr::Init(4, 40);
}

size_t ApplicationBase::TickCount() {
	return _total_tick_count_;
}

const std::string& ApplicationBase::ConfigFilePath()const {
	return _conf_file;
}

const std::string& ApplicationBase::AppName()const {
	return _appname;
}

const std::string& ApplicationBase::PidFile()const {
	return _pid_file;
}

const base::timestamp& ApplicationBase::GetNow()const {
	return _now;
}

int ApplicationBase::ParseOpt(int argc, char** argv) {
	_workdir = base::StringUtil::directory(argv[0]);
	_appname = base::StringUtil::basename(argv[0]);
	_appname = base::StringUtil::remove_from_end(_appname, ".exe");
#ifdef M_PLATFORM_WIN
#ifdef _DEBUG
	if (_appname.back() == 'd')
		_appname.pop_back();
#endif
#endif
	_pid_file = _appname + ".pid";

	if (argc < 2) {
		Usage();
		exit(0);
	}

	static int opt_char = 0;
	static base::Option long_option[] = {
		{"help", 0, 0, 'h'},
		{"daemon", 0, 0, 'D'},
		{"log_file", 1, &opt_char, 'L'},
		{"log_level", 1, &opt_char, 'l'},
		{"log_withpid", 1, &opt_char, 'p'},
		{"thread_cnt", 1, &opt_char, 'T'},
		{"conf_dir", 1, &opt_char, 'W'}
	};

	int opt_idx = 0;
	int opt;
	while (-1 != (opt = base::GetOptLong(argc, argv, "DHh", long_option, &opt_idx))) {
		switch (opt) {
		case 0:
			switch (*long_option[opt_idx].flag) {
			case 'L':
				_log_file = base::GetOptArg();
				break;
			case 'l':
				_log_level = base::GetOptArgI();
				break;
			case 'p':
				_log_withpid = base::GetOptArgI();
				break;
			case 'T':
				_svr_thread_cnt = base::GetOptArgI();
				break;
			case 'W':
				_confdir = base::GetOptArg();
				break;
			}
			break;
		case 'D':
			_daemon = 1;
			break;
		case 'H':
		case 'h':
		case '?':
		default:
			Usage();
			exit(0);
			break;
		}
	}
	
	if (_log_file.empty()) {
		_log_file = _appname;
	}

	_conf_file = _confdir + _appname + "/" + _appname + ".conf";
	_comm_conf_file = _confdir + "comm_conf/comm.conf";
	return 0;
}

void ApplicationBase::Usage()const {
	printf("Usage:\n");
	printf("required arguments:\n");
	printf("--config_file	the config file path\n");

	printf("\noptional arguments:\n");
	printf("--log_file		the log file path\n");
	printf("--log_level		the log level\n");
}

bool ApplicationBase::CheckReload() {
	if (TickCount() % (100 * 3) == 0) {
		static const char* filename = "_reload_";
		FILE *fp = myfopen(filename, "r", _SH_DENYNO);
		if (fp) {
			fclose(fp);
			remove(filename);
			return true;
		}
	}
	return false;
}

int ApplicationBase::SendMsgToSelf(int cmd, base::s_uint64_t uid, const google::protobuf::Message& msg) {
	// 暂时限制4k
	SelfMsg* buffer = SelfMsgAlloc::Alloc();
	
	AppHeadFrame* frame = (AppHeadFrame*)buffer->body;
	char* ser_buf = buffer->body + sizeof(AppHeadFrame);
	msg.SerializePartialToArray(ser_buf, SelfMsg::len - sizeof(AppHeadFrame));

	frame->set_is_broadcast(false);
	frame->set_src_svr_type(ServerType());
	frame->set_dst_svr_type(ServerType());
	frame->set_src_inst_id(InstanceId());
	frame->set_src_trans_id(0);
	frame->set_dst_trans_id(0);
	frame->set_cmd(cmd);
	frame->set_cmd_length(msg.ByteSize());
	frame->set_userid(uid);
	_self_msg_queue.push(buffer);
	return 0;
}

base::s_uint32_t ApplicationBase::PortStart() {
	if (ServerType() == proto::SVR_TYPE_ROUTER) {
		return _comm_config.Data().routersvr_port_start();
	}
	else if (ServerType() == proto::SVR_TYPE_CONN) {
		return _comm_config.Data().connsvr_port_start();
	} 

	LogError("failed to get start port of server_type:" << ServerType());
	return 0;
}

base::s_uint32_t ApplicationBase::CalcPort(int type) {
	return PortStart() + InstanceId();
}