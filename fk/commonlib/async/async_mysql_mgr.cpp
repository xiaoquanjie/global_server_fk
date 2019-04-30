#include "commonlib/async/async_mysql_mgr.h"
#include "mysqlclient/ma_wrapper.h"
#include "slience/base/string_util.hpp"
#include "slience/base/logger.hpp"

MysqlWork::MysqlWork() : _mutex(), _cond(_mutex) {

}

AsyncMysqlMgrImpl::AsyncMysqlMgrImpl() {
	_queue_max_size = 40;
}

int AsyncMysqlMgrImpl::Init(base::s_uint32_t work_cnt, base::s_uint32_t queue_max_size) {
	if (_work_thread.size()) {
		return 0;
	}

	_queue_max_size = queue_max_size;
	for (base::s_uint32_t idx = 0; idx < work_cnt; ++idx) {
		_work_queue.push_back(new MysqlWork);
		base::thread* thr = new base::thread(&AsyncMysqlMgrImpl::run, this, new base::s_uint32_t(idx));
		_work_thread.push_back(thr);
	}
	return 0;
}

MysqlRsp* AsyncMysqlMgrImpl::Pick() {
	MysqlRsp* rsp = 0;
	_mutex.lock();
	if (_rsp_queue.size()) {
		rsp = _rsp_queue.front();
		_rsp_queue.pop();
	}
	_mutex.unlock();
	return rsp;
}

void AsyncMysqlMgrImpl::run(void* param) {
	base::s_uint32_t* idx = (base::s_uint32_t*)param;
	MysqlWork* work = _work_queue[*idx];

	while (true) {
		work->_mutex.lock();
		if (work->_queue.empty()) {
			work->_cond.wait();
		}
		MysqlReq* req = 0;
		if (work->_queue.size()) {
			req = work->_queue.front();
			work->_queue.pop();
		}
		work->_mutex.unlock();
		work->_cond.notify();

		// 处于请求
		if (req) {
			MysqlRsp* rsp = proccss(req);
			delete req;

			// rsp丢入TransactionMgr进行处理
			_mutex.lock();
			_rsp_queue.push(rsp);
			_mutex.unlock();
		}
	}
}

bool AsyncMysqlMgrImpl::AddRequest(base::s_uint64_t orderid, MysqlReq* req) {
	// 队列满了只能等，没有别的办法
	if (_work_queue.empty()) {
		delete req;
		return false;
	}
	int idx = orderid % _work_queue.size();
	MysqlWork* work = _work_queue[idx];
	work->_mutex.lock();
	if (work->_queue.size() >= _queue_max_size) {
		work->_cond.wait();
	}
	work->_queue.push(req);
	work->_mutex.unlock();
	work->_cond.notify();
	return true;
}

MysqlRsp* AsyncMysqlMgrImpl::proccss(MysqlReq* req) {
	MysqlRsp* rsp = new MysqlRsp;
	rsp->trans_id = req->trans_id;
	rsp->err_code = 0;
	rsp->affected_rows = 0;
	rsp->mysql_res = 0;
	rsp->is_dup_entry = false;
	
	do {
		LogDebug("threadid:" << base::thread::ctid() << ", do sql request:" << req->sql);
		// 解析url
		std::vector<std::string> values;
		base::StringUtil::Split(req->url, ":", values);
		if (values.size() != 5) {
			rsp->err_code = -1;
			rsp->code_what = "illegal url";
			break;
		}

		unsigned short port = atoi(values[1].c_str());
		SqlConnectionPtr ptr = MysqlPool::GetConnection(values[0].c_str(), 
			values[2].c_str(), 
			values[3].c_str(),
			values[4].c_str(),
			port);
		if (!ptr) {
			rsp->err_code = -1;
			rsp->code_what = MysqlPool::GetErrorMsg();
			break;
		}

		if (req->type == MysqlReq::E_QUERY) {
			rsp->err_code = ptr->Execute(req->sql.c_str(), req->sql.length());
			if (rsp->err_code == 0) {
				rsp->mysql_res = ptr->StoreResult();
				rsp->affected_rows = (int)mysql_num_rows((MYSQL_RES *)rsp->mysql_res);
			}
		}
		else {
			rsp->err_code = ptr->Execute(req->sql.c_str(), req->sql.length(), rsp->affected_rows);
		}
		if (rsp->err_code != 0) {
			rsp->code_what = ptr->GetErrorMsg();
			rsp->is_dup_entry = ptr->IsDupEntry();
		}

	} while (false);
	return rsp;
}

//////////////////////////////////////////////////////////

int AsyncMysqlMgr::Init(base::s_uint32_t work_cnt, base::s_uint32_t queue_max_size) {
	return GetImpl()->Init(work_cnt, queue_max_size);
}

MysqlRsp* AsyncMysqlMgr::Pick() {
	return GetImpl()->Pick();
}

bool AsyncMysqlMgr::AddRequest(base::s_uint64_t orderid, MysqlReq* req) {
	return GetImpl()->AddRequest(orderid, req);
}

AsyncMysqlMgrImpl* AsyncMysqlMgr::GetImpl() {
	static AsyncMysqlMgrImpl impl;
	return &impl;
}