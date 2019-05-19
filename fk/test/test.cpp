#include "test/test.h"
#include "commonlib/transaction/transaction_mgr.h"
#include "commonlib/net_handler/net_handler.h"
#include "protolib/src/mytest.pb.h"

int TestApplication::ServerType() {
	return 0;
}

int TestApplication::InstanceId() {
	return 0;
}

int TestApplication::ServerZone() {
	return 0;
}

int TestApplication::OnInitNetWork() {
	// start network thread
	auto func = m_bind_t(&TestApplication::OnProc, this, placeholder_1,
		placeholder_2, placeholder_3, placeholder_4);
	NetIoHandlerSgl.Init(_now, func);
	NetIoHandlerSgl.Start(_svr_thread_cnt, false);
	return 0;
}

void TestApplication::OnStopNetWork() {
	NetIoHandlerSgl.Stop();
}

int TestApplication::UpdateNetWork() {
	return NetIoHandlerSgl.Update();
}

bool TestApplication::UseAsyncMysql() {
	return true;
}

bool TestApplication::UseAsyncRedis() {
	return true;
}

int TestApplication::OnTick(const base::timestamp& now) {
	static int idx = 0;
	if (false) {
		if (TickCount() % 10000 == 0) {
			mytest::MysqlTestNotify notify;
			SendMsgToSelf(mytest::CMD::MYSQL_TEST_NOTIFY, idx, notify);
			idx++;
		}
	}

	if (true) {
		if (TickCount() % 10000 == 0) {
			mytest::RedisTestNotify notify;
			SendMsgToSelf(mytest::CMD::REDIS_TEST_NOTIFY, idx, notify);
			idx++;
		}
	}
	
	return 0;
}