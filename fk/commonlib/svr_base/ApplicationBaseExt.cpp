#include "commonlib/svr_base/ApplicationBase.h"
#include "protolib/src/svr_base.pb.h"
#include "slience/base/logger.hpp"
#include "commonlib/svr_base/ApplicationFunc.hpp"
#include "commonlib/transaction/transaction_mgr.h"
#include "commonlib/async/async_mysql_mgr.h"
#include "commonlib/async/async_redis_mgr.h"

int ApplicationBase::Run() {
	while (!gAppExist) {
		base::timestamp t_now;
		if (t_now.millisecond() > _now.millisecond()) {
			_total_tick_count_++;
			_now = t_now;
			TransactionMgr::Update(_now);
			OnTick(_now);
		}
		if (CheckReload()) {
			LogInfo("reload begin.................");
			OnReload();
			LogInfo("reload end....................");
		}
		if (0 != UpdateNetWork()) {
			Sleep(1);
		}
		if (UseAsyncMysql()) {
			MysqlRsp* rsp = AsyncMysqlMgr::Pick();
			if (rsp) {
				TransactionMgr::ProcessMysqlRsp(rsp);
				delete rsp;
			}
		}
		if (UseAsyncRedis()) {
			RedisRsp* rsp = AsyncRedisMgr::Pick();
			if (rsp) {
				TransactionMgr::ProcessRedisRsp(rsp);
				delete rsp;
			}
		}
		if (_self_msg_queue.size()) {
			auto* msg = _self_msg_queue.front();
			_self_msg_queue.pop();
			AppHeadFrame& frame = (AppHeadFrame&)*msg;
			TransactionMgr::ProcessFrame(0, ServerType(), InstanceId(), frame, msg + sizeof(AppHeadFrame));
			delete[]msg;
		}
	}
	OnExit();
	LogInfo("application exit..................");
	return 0;
}
