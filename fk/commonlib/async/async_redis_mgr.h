#pragma once

#include "slience/base/thread.hpp"
#include "slience/base/mutexlock.hpp"
#include "slience/base/condition.hpp"
#include <queue>
#include "commonlib/common/comm_struct.h"

struct RedisWork {
	base::MutexLock _mutex;
	base::Condition _cond;
	std::queue<RedisReq*> _queue;

	RedisWork();
};

class AsyncRedisMgrImpl {
public:
	AsyncRedisMgrImpl();

	int Init(base::s_uint32_t work_cnt, base::s_uint32_t queue_max_size);

	RedisRsp* Pick();

	bool AddRequest(base::s_uint64_t orderid, RedisReq* req);

protected:
	void run(void* param);

	RedisRsp* proccss(RedisReq* req);

private:
	base::s_uint32_t _queue_max_size;
	std::vector<RedisWork*> _work_queue;
	std::vector<base::thread*> _work_thread;

	base::MutexLock _mutex;
	std::queue<RedisRsp*> _rsp_queue;
};

class AsyncRedisMgr {
public:
	static int Init(base::s_uint32_t work_cnt, base::s_uint32_t queue_max_size);

	static RedisRsp* Pick();

	static bool AddRequest(base::s_uint64_t orderid, RedisReq* req);
protected:
	static AsyncRedisMgrImpl* GetImpl();

};