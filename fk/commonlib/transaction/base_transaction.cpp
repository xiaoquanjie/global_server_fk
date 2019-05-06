#include "commonlib/transaction/base_transaction.h"
#include "slience/coroutine/coroutine.hpp"
#include "commonlib/transaction/transaction_mgr.h"
#include "slience/base/buffer.hpp"
#include "commonlib/net_handler/net_handler.h"
#include "slience/base/random.hpp"
#include "commonlib/async/async_mysql_mgr.h"
#include "mysqlclient/ma_wrapper.h"
#include "commonlib/async/async_redis_mgr.h"
#include "hiredis_wrapper/wrapper/redis_wrapper.hpp"

void Transaction::Construct() {
	_trans_id = 0;
	_userid = 0;
	_co_id = 0;
	_fd = 0;
	_cur_fd = 0;
	_cur_frame = 0;
	_cur_frame_data = 0;
	_self_svr_type = 0;
	_self_inst_id = 0;
	_self_server_zone = 0;
	_state = E_STATE_IDLE;
	_timer_id = 0;
	_req_random = 0;
	_mysql_rsp = 0;
	_redis_rsp = 0;
	memset(&_ori_frame, 0, sizeof(_ori_frame));
}

Transaction::Transaction(int cmd) {
	_cmd = cmd;
	Construct();
}

Transaction::~Transaction() {

}

int Transaction::ParseMsg(google::protobuf::Message& message) {
	if (!_cur_frame_data || !_cur_frame) {
		assert(0);
		LogError("userid: " << userid() << " cmd: " << cmd() << "_cur_frame_data or _cur_frame is nullptr");
		return -1;
	}
	bool ret = message.ParseFromArray(_cur_frame_data, _cur_frame->get_cmd_length());
	if (ret) {
		return 0;
	}
	else {
		LogError("userid: " << userid() << " cmd: " << cmd() << " parse message fail, len:" << _cur_frame->get_cmd_length());
		return -1;
	}
}

int Transaction::Process(base::s_int64_t fd, base::s_uint32_t self_svr_type,
	base::s_uint32_t self_inst_id, base::s_uint32_t self_server_zone,
	const AppHeadFrame& frame, const char* data) {
	if (req_random() != 0) {
		if (req_random() != frame.get_req_random()) {
			LogWarn(frame.ToString() << " is not waiting cmd packet");
			return -1;
		}
	}
	if (!_cur_frame_data) {
		_userid = frame.get_userid();
		_cmd = frame.get_cmd();
		_fd = fd;
		_self_svr_type = self_svr_type;
		_self_inst_id = self_inst_id;
		_self_server_zone = self_server_zone;
		_ori_frame = frame;
	}
	else {
		_state = E_STATE_ACTIVE;
	}
	_cur_frame_data = data;
	_cur_frame = &frame;
	_cur_fd = fd;
	OnState();
	return 0;
}

int Transaction::ProcessMysqlRsp(void* rsp) {
	_mysql_rsp = rsp;
	_state = E_STATE_ACTIVE;
	OnState();
	return 0;
}

int Transaction::ProcessRedisRsp(void* rsp) {
	_redis_rsp = rsp;
	_state = E_STATE_ACTIVE;
	OnState();
	return 0;
}

void Transaction::OnState() {
	CancelTimer();
	int ret = 0;
	switch (_state) {
	case E_STATE_IDLE:
		ret = OnIdle();
		break;
	case E_STATE_ACTIVE:
		ret = OnActive();
		break;
	case E_STATE_TIMEOUT:
		ret = OnTimeOut();
		break;
	}
}

void Transaction::CancelTimer() {
	if (0 != _timer_id) {
		if (0 != TransactionMgr::CancelTimer(_timer_id)) {
			LogError("cancel timer fail, id: " << _timer_id);
		}
		_timer_id = 0;
	}
}

void Transaction::SetTimer(int interval) {
	CancelTimer();
	base::s_uint64_t id = TransactionMgr::AddTimer(interval, trans_id());
	if (id == 0) {
		LogError("add timer fail, interval: " << interval);
		return;
	}
}

Transaction::Wait_Return Transaction::Wait(int interval) {
	if (0 != co_id()) {
		SetTimer(interval);
		coroutine::Coroutine::yield();
		int state = _state;
		_state = E_STATE_IDLE;

		if (state == E_STATE_ACTIVE) {
			return E_RETURN_ACTIVE;
		}
		else {
			return E_RETURN_TIMEOUT;
		}
	}
	else {
		// don't allow
		LogError("don't call the wait interface on main coroutine directly or indirectly");
		throw int(-1);
		return E_RETURN_ERROR;
	}
}

int Transaction::OnIdle() {
	// 分配trans_id
	_trans_id = TransactionMgr::GeneratorTransId();
	// 启动协程
	coroutine::CoroutineTask::doTask(&TransactionMgr::CoroutineEnter, (void*)this);
	return 0;
}

int Transaction::OnActive() {
	// 唤醒协程
	if (0 != co_id()) {
		coroutine::CoroutineTask::resumeTask(co_id());
	}
	return 0;
}

int Transaction::OnTimeOut() {
	// 唤醒协程
	_timer_id = 0;
	if (0 != co_id()) {
		set_req_random(0);
		coroutine::CoroutineTask::resumeTask(co_id());
	}
	return 0;
}

int Transaction::SendMsgByServerType(int cmd, int svr_type,
	google::protobuf::Message& request, google::protobuf::Message& respond) {
	if (0 != SendMsgByServerType(cmd, svr_type, request)) {
		return -1;
	}
	Wait_Return ret = Wait(E_WAIT_FIVE_SECOND);
	if (ret == E_RETURN_TIMEOUT) {
		LogError(
			"{userid:" << userid() << 
			" svr_type:"<< svr_type <<
			"} Timeout to wait response of SendMsgByServerType");
		return -1;
	}
	else if (ret == E_RETURN_ERROR) {
		LogError(
			"{userid:" << userid() <<
			" svr_type:" << svr_type <<
			"} Error to wait response of SendMsgByServerType");
		return -1;
	}
	
	// parse msg
	if (0 != ParseMsg(respond)) {
		LogError(request.GetTypeName() << ".ParseFromArray fail");
		return -1;
	}
	return 0;
}

int Transaction::SendMsgByServerType(int cmd, int svr_type,
	google::protobuf::Message& request) {
	set_req_random(base::random().rand(10000, 100000));
	int ret = RouterMgrSgl.SendMsg(cmd,
		userid(),
		false,
		svr_type,
		0,
		trans_id(),
		0,
		req_random(),
		request
	);
	return ret;
}

int Transaction::SendMsgByServerId(int cmd, int svr_type, int inst_id,
	google::protobuf::Message& request, google::protobuf::Message& respond) {
	if (0 != SendMsgByServerId(cmd, svr_type, inst_id, request)) {
		return -1;
	}
	Wait_Return ret = Wait(E_WAIT_FIVE_SECOND);
	if (ret == E_RETURN_TIMEOUT) {
		LogError(
			"{userid:"   << userid() <<
			" svr_type:" << svr_type <<
			" inst_id:"  << inst_id  <<
			"} Timeout to wait response of SendMsgByServerId");
		return -1;
	}
	else if (ret == E_RETURN_ERROR) {
		LogError(
			"{userid:" << userid() <<
			" svr_type:" << svr_type <<
			"} Error to wait response of SendMsgByServerId");
		return -1;
	}

	// parse msg
	if (0 != ParseMsg(respond)) {
		LogError(request.GetTypeName() << ".ParseFromArray fail");
		return -1;
	}
	return 0;
}

int Transaction::SendMsgByServerId(int cmd, int svr_type, int inst_id,
	google::protobuf::Message& request) {
	set_req_random(base::random().rand(10000, 100000));
	int ret = RouterMgrSgl.SendMsg(cmd,
		userid(),
		false,
		svr_type,
		inst_id,
		trans_id(),
		0,
		req_random(),
		request
	);
	return ret;
}

int Transaction::SendMsgByFd(int cmd, google::protobuf::Message& request) {
	set_req_random(base::random().rand(10000, 100000));
	int ret = RouterMgrSgl.SendMsgByFd(fd(),
		cmd,
		userid(),
		false,
		0,
		0,
		trans_id(),
		0,
		req_random(),
		request);
	return ret;
}

int Transaction::SendMsgByFd(int cmd, google::protobuf::Message& request
	, google::protobuf::Message& respond) {
	if (0 != SendMsgByFd(cmd, request)) {
		return -1;
	}
	Wait_Return ret = Wait(E_WAIT_FIVE_SECOND);
	if (ret == E_RETURN_TIMEOUT) {
		LogError(
			"{userid:" << userid() <<
			" fd:" << fd() <<
			"} Timeout to wait response of SendMsgByFd");
		return -1;
	}
	else if (ret == E_RETURN_ERROR) {
		LogError(
			"{userid:" << userid() <<
			" fd:" << fd() <<
			"} Error to wait response of SendMsgByFd");
		return -1;
	}

	// parse msg
	if (0 != ParseMsg(respond)) {
		LogError(request.GetTypeName() << ".ParseFromArray fail");
		return -1;
	}
	return 0;
}

// 需要注意一下返回值
int Transaction::MysqlQuery(base::s_uint64_t orderid,
	const std::string& url,
	const std::string& sql,
	int expected_fields,
	MysqlCallBack func) {
	int affected_rows = 0;
	return MysqlQuery(orderid, url, sql, affected_rows, expected_fields, func);
}

int Transaction::MysqlQuery(base::s_uint64_t orderid,
	const std::string& url,
	const std::string& sql,
	int& affected_rows,
	int expected_fields,
	MysqlCallBack func) {
	MysqlReq* req = new MysqlReq;
	req->type = MysqlReq::E_QUERY;
	req->trans_id = trans_id();
	req->url = url;
	req->sql = sql;

	if (!AsyncMysqlMgr::AddRequest(orderid, req)) {
		LogError("failed to add request to AsyncMysqlMgr");
		return E_MYSQL_ERROR;
	}

	Wait(E_WAIT_MYSQL);
	MysqlRsp* rsp = (MysqlRsp*)_mysql_rsp;
	if (!rsp) {
		LogError("MysqlRsp is null");
		return E_MYSQL_ERROR;
	}

	int ret = E_MYSQL_SUCCESS;
	do {
		if (rsp->err_code != 0) {
			LogError(
				"{userid:"
				<< userid()
				<< "} "
				<< "failed to Request Mysql, code:"
				<< rsp->err_code
				<< " ,is_dup_key:"
				<< rsp->is_dup_entry
				<< " ,what:"
				<< rsp->code_what
				<< " ,sql:"
				<< sql;
			);
			// 重键错误　
			ret = rsp->is_dup_entry ? E_MYSQL_DUP_KEY : E_MYSQL_ERROR;
			break;
		}

		MYSQL_RES* result = (MYSQL_RES *)rsp->mysql_res;
		if (expected_fields != (int)mysql_num_fields(result)) {
			LogError("{userid:"
				<< userid()
				<< "} failed to Request Mysql, wrong expected_fields:"
				<< expected_fields
				<< " ,real_fields:"
				<< mysql_num_fields(result)
				<< " ,sql:"
				<< sql;
			);
			ret = E_MYSQL_ERROR;
			break;
		}

		affected_rows = rsp->affected_rows;
		int idx = 0;
		MYSQL_ROW row = 0;
		while (0 != (row = mysql_fetch_row(result))) {
			func(idx, row);
		}
	} while (false);

	_mysql_rsp = 0;
	mysql_free_result((MYSQL_RES *)rsp->mysql_res);
	return ret;
}

int Transaction::RedisExecute(base::s_uint16_t orderid,
	const std::string& url,
	const BaseRedisCmd& cmd,
	RedisCallBack func) {
	RedisReq* req = new RedisReq;
	req->trans_id = trans_id();
	req->cmd = cmd.cmd;
	req->url = url;

	if (!AsyncRedisMgr::AddRequest(orderid, req)) {
		LogError("failed to add request to AsyncRedisMgr");
		return E_REDIS_ERROR;
	}

	Wait(E_WAIT_REDIS);
	RedisRsp* rsp = (RedisRsp*)_redis_rsp;
	if (!rsp) {
		LogError("RedisRsp is null");
		return E_REDIS_ERROR;
	}

	RedisReplyParser parser((redisReply*)rsp->reply);
	if (rsp->err_code != 0) {
		LogError(
			"{userid:"
			<< userid()
			<< "} failed to Request Redis, code"
			<< rsp->err_code
			<< " ,what:"
			<< rsp->code_what
			<< " ,cmd:"
			<< cmd.GetCmd()
		);
		return E_REDIS_ERROR;
	}

	_redis_rsp = 0;
	func(parser);
	return 0;
}

base::s_uint32_t Transaction::trans_id() {
	return _trans_id;
}

base::s_uint32_t Transaction::cmd() {
	return _cmd;
}

base::s_uint64_t Transaction::userid() {
	return _userid;
}

base::s_int32_t Transaction::co_id() {
	return _co_id;
}

base::s_int64_t Transaction::fd() {
	return _fd;
}

base::s_int64_t Transaction::cur_fd() {
	return _cur_fd;
}

base::s_uint32_t Transaction::self_svr_type() {
	return _self_svr_type;
}

base::s_uint32_t Transaction::self_inst_id() {
	return _self_inst_id;
}

base::s_uint32_t Transaction::self_server_zone() {
	return _self_server_zone;
}

void Transaction::set_co_id(base::s_int32_t id) {
	_co_id = id;
}

const AppHeadFrame& Transaction::cur_frame() {
	return *_cur_frame;
}

const AppHeadFrame& Transaction::ori_frame() {
	return _ori_frame;
}

void Transaction::set_req_random(base::s_uint32_t r) {
	_req_random = r;
}

base::s_uint32_t Transaction::req_random() {
	return _req_random;
}
