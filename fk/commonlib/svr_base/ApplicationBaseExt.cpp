#include "commonlib/svr_base/ApplicationBase.h"
#include "protolib/src/svr_base.pb.h"
#include "slience/base/logger.hpp"
#include "commonlib/svr_base/ApplicationFunc.hpp"
#include "commonlib/transaction/transaction_mgr.h"
#include "commonlib/async/async_mysql_mgr.h"
#include "commonlib/async/async_redis_mgr.h"
#include "commonlib/svr_base/svralloc.h"

int ApplicationBase::Run() {
	const int max_idle_count = 100;
	int idle_count = 0;
	while (!gAppExist) {
		bool is_idle = true;
		base::timestamp t_now;
		if ((t_now.millisecond() - _now.millisecond()) >= 10) {
			// 10毫秒一跳
			_total_tick_count += 1;
			_now = t_now;
			OnTick(_now);
			PrintStatus();
			if (CheckReload()) {
				LogInfo("reload begin.................");
				OnReload();
				LogInfo("reload end....................");
			}
		}
		TransactionMgr::Update(_now);
		if (0 == UpdateNetWork()) {
			is_idle = false;
		}
		if (UseAsyncMysql()) {
			MysqlRsp* rsp = AsyncMysqlMgr::Pick();
			if (rsp) {
				is_idle = false;
				TransactionMgr::ProcessMysqlRsp(rsp);
				delete rsp;
			}
		}
		if (UseAsyncRedis()) {
			RedisRsp* rsp = AsyncRedisMgr::Pick();
			if (rsp) {
				is_idle = false;
				TransactionMgr::ProcessRedisRsp(rsp);
				delete rsp;
			}
		}
		if (UseZookeeper()) {
			_zkconn_mgr.update();
		}
		while (_self_msg_queue.size()) {
			is_idle = false;
			auto* msg = _self_msg_queue.front();
			_self_msg_queue.pop();
			AppHeadFrame& frame = (AppHeadFrame&)*msg;
			TransactionMgr::ProcessFrame(0, 
				ServerType(), 
				InstanceId(),
				ServerZone(),
				frame, 
				msg->body + sizeof(AppHeadFrame));
			SelfMsgAlloc::Dealloc(msg);
		}
		if (is_idle) {
			idle_count++;
		}
		if (idle_count >= max_idle_count) {
			Sleep(1);
			idle_count = 0;
		} 
	}
	OnExit();
	LogInfo("application exit..................");
	return 0;
}

void ApplicationBase::PrintStatus() {
	if (TickCount() % (100 * 2 * 60) == 0) {
		LogInfo("Application status -----> TcpSocketMsgAlloc alloc_num:"
			<< TcpSocketMsgAlloc::GetCount()
			<< " alloc_size:"
			<< TcpSocketMsgAlloc::GetAllocSize()
		);
		LogInfo("Application status -----> TcpConnectorMsgAlloc alloc_num:"
			<< TcpConnectorMsgAlloc::GetCount()
			<< " alloc_size:"
			<< TcpConnectorMsgAlloc::GetAllocSize()
		);
		LogInfo("Application status -----> SelfMsgAlloc alloc_num:"
			<< SelfMsgAlloc::GetCount()
			<< " alloc_size:"
			<< SelfMsgAlloc::GetAllocSize()
		);
	}
}