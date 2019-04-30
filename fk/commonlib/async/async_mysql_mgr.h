#pragma once

#include "slience/base/thread.hpp"
#include "slience/base/mutexlock.hpp"
#include "slience/base/condition.hpp"
#include <vector>
#include <queue>
#include "commonlib/common/comm_struct.h"


struct MysqlWork {
	base::MutexLock _mutex;
	base::Condition _cond;
	std::queue<MysqlReq*> _queue;

	MysqlWork();
};

class AsyncMysqlMgrImpl {
public:
	AsyncMysqlMgrImpl();

	int Init(base::s_uint32_t work_cnt, base::s_uint32_t queue_max_size);

	MysqlRsp* Pick();

	bool AddRequest(base::s_uint64_t orderid, MysqlReq* req);

protected:
	void run(void* param);

	MysqlRsp* proccss(MysqlReq* req);
private:
	base::s_uint32_t _queue_max_size;
	std::vector<MysqlWork*> _work_queue;
	std::vector<base::thread*> _work_thread;

	base::MutexLock _mutex;
	std::queue<MysqlRsp*> _rsp_queue;
};

//////////////////////////////////////////////////////////

class AsyncMysqlMgr {
public:
	static int Init(base::s_uint32_t work_cnt, base::s_uint32_t queue_max_size);

	static MysqlRsp* Pick();

	static bool AddRequest(base::s_uint64_t orderid, MysqlReq* req);
protected:
	static AsyncMysqlMgrImpl* GetImpl();

};

