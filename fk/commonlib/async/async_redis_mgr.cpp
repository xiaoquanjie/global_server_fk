#include "commonlib/async/async_redis_mgr.h"
#include "slience/base/string_util.hpp"
#include "slience/base/logger.hpp"
#include "hiredis_wrapper/wrapper/redis_wrapper.hpp"

RedisWork::RedisWork() : _mutex(), _cond(_mutex) {

}

AsyncRedisMgrImpl::AsyncRedisMgrImpl() {
	_queue_max_size = 40;
}

int AsyncRedisMgrImpl::Init(base::s_uint32_t work_cnt, base::s_uint32_t queue_max_size) {
	if (_work_thread.size()) {
		return 0;
	}

	_queue_max_size = queue_max_size;
	for (base::s_uint32_t idx = 0; idx < work_cnt; ++idx) {
		_work_queue.push_back(new RedisWork);
		base::thread* thr = new base::thread(&AsyncRedisMgrImpl::run, this, new base::s_uint32_t(idx));
		_work_thread.push_back(thr);
	}
	return 0;
}

RedisRsp* AsyncRedisMgrImpl::Pick() {
	RedisRsp* rsp = 0;
	_mutex.lock();
	if (_rsp_queue.size()) {
		rsp = _rsp_queue.front();
		_rsp_queue.pop();
	}
	_mutex.unlock();
	return rsp;
}

void AsyncRedisMgrImpl::run(void* param) {
	base::s_uint32_t* idx = (base::s_uint32_t*)param;
	RedisWork* work = _work_queue[*idx];

	while (true) {
		work->_mutex.lock();
		if (work->_queue.empty()) {
			work->_cond.wait();
		}
		RedisReq* req = 0;
		if (work->_queue.size()) {
			req = work->_queue.front();
			work->_queue.pop();
		}
		work->_mutex.unlock();
		work->_cond.notify();

		// 处于请求
		if (req) {
			RedisRsp* rsp = proccss(req);
			delete req;

			// rsp丢入TransactionMgr进行处理
			_mutex.lock();
			_rsp_queue.push(rsp);
			_mutex.unlock();
		}
	}
}

bool AsyncRedisMgrImpl::AddRequest(base::s_uint64_t orderid, RedisReq* req) {
	// 队列满了只能等，没有别的办法
	if (_work_queue.empty()) {
		delete req;
		return false;
	}
	int idx = orderid % _work_queue.size();
	RedisWork* work = _work_queue[idx];
	work->_mutex.lock();
	if (work->_queue.size() >= _queue_max_size) {
		work->_cond.wait();
	}
	work->_queue.push(req);
	work->_mutex.unlock();
	work->_cond.notify();
	return true;
}

RedisRsp* AsyncRedisMgrImpl::proccss(RedisReq* req) {
	RedisRsp* rsp = new RedisRsp;
	rsp->trans_id = req->trans_id;
	rsp->reply = 0;
	rsp->err_code = 0;
	
	try {
		do {
			// 解析url
			std::vector<std::string> values;
			base::StringUtil::Split(req->url, ":", values);
			if (values.size() != 5) {
				rsp->err_code = -1;
				rsp->code_what = "illegal url";
				break;
			}

			unsigned short port = atoi(values[1].c_str());
			unsigned short db = atoi(values[4].c_str());
			RedisConnection conn = RedisPool::GetConnection(values[0],
				port,
				db,
				values[2],
				values[3]);

			rsp->reply = conn.Command(BaseRedisCmd(req->cmd));
		} while (false);
	}
	catch (RedisException& e) {
		rsp->err_code = e.Code();
		rsp->code_what = e.What();
	}
	return rsp;
}

//////////////////////////////////////////////////////////

int AsyncRedisMgr::Init(base::s_uint32_t work_cnt, base::s_uint32_t queue_max_size) {
	return GetImpl()->Init(work_cnt, queue_max_size);
}

RedisRsp* AsyncRedisMgr::Pick() {
	return GetImpl()->Pick();
}

bool AsyncRedisMgr::AddRequest(base::s_uint64_t orderid, RedisReq* req) {
	return GetImpl()->AddRequest(orderid, req);
}

AsyncRedisMgrImpl* AsyncRedisMgr::GetImpl() {
	static AsyncRedisMgrImpl impl;
	return &impl;
}