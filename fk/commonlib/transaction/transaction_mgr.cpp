#include "commonlib/transaction/transaction_mgr.h"
#include "slience/coroutine/coroutine.hpp"
#include "slience/base/logger.hpp"
#include "commonlib/transaction/base_transaction.h"

TransactionBucket::TransactionBucket(base::s_uint32_t cmd) {
	_cmd = cmd;
}

TransactionBucket::~TransactionBucket() {
	for (auto iter = _trans_vec.begin(); iter != _trans_vec.end(); ++iter) {
		delete (*iter);
	}
	_trans_vec.clear();
}

TransactionMgrImpl::TransactionMgrImpl() {
	_max_concurrent_trans = 5000;
	_trans_id_generator = 1;
}

void TransactionMgrImpl::Init() {
	coroutine::Coroutine::init(128 * 1024, true);
}

void TransactionMgrImpl::Init(base::s_int32_t max_concurrent_trans) {
	_max_concurrent_trans = max_concurrent_trans;
	Init();
}

void TransactionMgrImpl::Update(const base::timestamp& now) {
	_timer_pool.Update(now);
	if (now.second() - _last_check_time.second() > 60) {
		_last_check_time = now;
		PrintStatus();
	}
}

int TransactionMgrImpl::ProcessFrame(base::s_int64_t fd, base::s_uint32_t self_svr_type,
	base::s_uint32_t self_inst_id,
	const AppHeadFrame& frame, const char* data) {
	if (frame.get_dst_trans_id() > 0) {
		Transaction* p = GetTransaction(frame.get_dst_trans_id());
		if (p) {
			p->Process(fd, self_svr_type, self_inst_id, frame, data);
		}
		else {
			LogWarn("can't find the old transaction, trans_id: " << frame.get_dst_trans_id());
			return -1;
		}
	}
	else {
		Transaction* p = CreateTransaction(frame.get_userid(), frame.get_cmd());
		if (p) {
			p->Process(fd, self_svr_type, self_inst_id, frame, data);
		}
		else {
			LogError("can't create new transaction, cmd: " << frame.get_cmd());
			return -1;
		}
	}
	return 0;
}

int TransactionMgrImpl::ProcessMysqlRsp(MysqlRsp* rsp) {
	do {
		Transaction* p = GetTransaction(rsp->trans_id);
		if (p) {
			p->ProcessMysqlRsp(rsp);
		}
		else {
			LogWarn("can't find the old transaction, trans_id: " << rsp->trans_id);
		}
	} while (false);
	return 0;
}

int TransactionMgrImpl::ProcessRedisRsp(RedisRsp* rsp) {
	do {
		Transaction* p = GetTransaction(rsp->trans_id);
		if (p) {
			p->ProcessRedisRsp(rsp);
		}
		else {
			LogWarn("can't find the old transaction, trans_id: " << rsp->trans_id);
		}
	} while (false);
	return 0;
}

void TransactionMgrImpl::CoroutineEnter(void* p) {
	Transaction* trans = (Transaction*)p;
	trans->set_co_id(coroutine::Coroutine::curid());
	_active_trans_map.insert(std::make_pair(trans->trans_id(), trans));
	
	LogInfo(
		"run a new transaction: "
		<< "co_id£º"
		<< trans->co_id()
		<< " trans_id: "
		<< trans->trans_id()
	);
	trans->InCoroutine();
	LogInfo(
		"end a transaction: "
		<< "co_id£º"
		<< trans->co_id()
		<< " trans_id: "
		<< trans->trans_id()
	);

	_active_trans_map.erase(trans->trans_id());
	RecyleTransaction(trans);
}

void TransactionMgrImpl::TimerCallback(base::s_uint32_t trans_id) {
	auto trans = GetTransaction(trans_id);
	if (trans) {
		trans->_state = Transaction::E_STATE_TIMEOUT;
		trans->OnState();
	}
}

int TransactionMgrImpl::CancelTimer(base::s_uint64_t id) {
	return _timer_pool.CancelTimer(id);
}

base::s_uint64_t TransactionMgrImpl::AddTimer(int interval, base::s_uint32_t trans_id) {
	m_function_t<void()> cb = m_bind_t(&TransactionMgrImpl::TimerCallback, this, trans_id);
	return _timer_pool.AddTimer(interval, cb);
}

base::s_uint32_t TransactionMgrImpl::GeneratorTransId() {
	return _trans_id_generator++;
}

Transaction* TransactionMgrImpl::CreateTransaction(base::s_uint64_t uid, base::s_uint32_t cmd) {
	if (GetActiveTransCnt() > _max_concurrent_trans) {
		LogError("create transaction fail, because of the max concurrency cnt:" << _max_concurrent_trans);
		return NULL;
	}

	auto iter = _trans_bucket_map.find(cmd);
	if (iter != _trans_bucket_map.end()) {
		return iter->second->CreateTransaction();
	}
	else {
		LogError("can't find the transaction bucket in trans_bucket_map, cmd: " << cmd);
		return NULL;
	}
}

Transaction* TransactionMgrImpl::GetTransaction(base::s_uint32_t trans_id) {
	auto iter = _active_trans_map.find(trans_id);
	if (iter != _active_trans_map.end()) {
		return iter->second;
	}
	else {
		return NULL;
	}
}

void TransactionMgrImpl::RecyleTransaction(Transaction* trans) {
	if (!trans) {
		return;
	}
	auto iter = _trans_bucket_map.find(trans->cmd());
	if (iter != _trans_bucket_map.end()) {
		iter->second->RecycleTransaction(trans);
	}
	else {
		LogError("can't find the transaction bucket in trans_bucket_map, cmd: " << trans->cmd());
	}
}

void TransactionMgrImpl::PrintStatus() {
	LogInfo("transaction_mgr status, active_trans_cnt:" << GetActiveTransCnt());
	int cmd_cnt = 0;
	int trans_cnt = 0;
	for (auto iter = _trans_bucket_map.begin(); iter != _trans_bucket_map.end(); ++iter) {
		cmd_cnt++;
		trans_cnt += iter->second->Size();
	}
	LogInfo("transaction_mgr status, nonactive_trans_cnt: " << trans_cnt);
	LogInfo("transaction_mgr status, cmd_cnt:" << cmd_cnt);
}

base::s_int32_t TransactionMgrImpl::GetActiveTransCnt() {
	return _active_trans_map.size();
}

///////////////////////////////////////////////////////////////////////

void TransactionMgr::Init() {
	GetImpl()->Init();
}
