#include "protolib/src/svr_base.pb.h"
#include "protolib/src/cmd.pb.h"
#include "commonlib/transaction/base_transaction.h"
#include "commonlib/transaction/transaction_mgr.h"
#include "commonlib/net_handler/net_handler.h"
#include "transfersvr/router_inst_mgr.h"

class TransClientIn 
	: public BaseTransaction<TransClientIn, proto::SocketClientIn> {
public:
	TransClientIn(unsigned int cmd) : BaseTransaction(cmd) {}

	int OnRequest(proto::SocketClientIn& request) {
		// 暂无处理
		return 0;
	}

};

REGISTER_TRANSACTION(CMD_SOCKET_CLIENT_IN, TransClientIn);

///////////////////////////////////////////////////////////////////////////

class TransClientOut
	: public BaseTransaction<TransClientOut, proto::SocketClientOut> {
public:
	TransClientOut(unsigned int cmd) : BaseTransaction(cmd) {}

	int OnRequest(proto::SocketClientOut& request) {
		if (M_CHECK_IS_TCP_CONNECTOR_FD(fd())) {
			netiolib::TcpConnectorPtr ptr = NetIoHandlerSgl.GetConnectorPtr(fd());
			if (!ptr) {
				return 0;
			}

			if (ptr->GetListenConnType() == Enum_ConnType_Transfer2) {
				// 主动的transfe的连接
				OnActiveTransfer();
			}
		}
		else {
			netiolib::TcpSocketPtr ptr = NetIoHandlerSgl.GetSocketPtr(fd());
			if (!ptr) {
				return 0;
			}
			if (ptr->GetListenConnType() == Enum_ListenType_Transfer) {
				// router的连接
				OnRouter();
			}
			else {
				// 被动的transfer的连接
				OnPasiveTransfer();
			}
		}
		return 0;
	}

	void OnRouter() {
		RouterInstanceMgrSql.LogoutInstance(fd());
	}

	void OnPasiveTransfer() {

	}

	void OnActiveTransfer() {

	}
};

REGISTER_TRANSACTION(CMD_SOCKET_CLIENT_OUT, TransClientOut);

///////////////////////////////////////////////////////////////////////////

class TransRegisterServer 
	: public BaseTransaction<TransRegisterServer, proto::RegisterServerReq> {
public:
	TransRegisterServer(unsigned int cmd) : BaseTransaction(cmd) {}

	int OnRequest(proto::RegisterServerReq& request) {
		// 相同服务区才允许注册
		int ret_code = 0;
		do {
			if ((base::s_uint32_t)request.server_zone() != self_svr_zone()) {
				ret_code = -1;
				LogWarn("register router failed, because of different svr_zone, self_zone="
					<< self_svr_zone()
					<< " request_zone="
					<< request.server_zone()
				);
				break;
			}

			ret_code = RouterInstanceMgrSql.LoginInstance(request.server_type(), request.instance_id(), fd());

		} while (false);

		proto::RegisterServerRsp respond;
		respond.mutable_ret()->set_code(ret_code);

		_frame.set_is_broadcast(false);
		_frame.set_src_zone(self_svr_zone());
		_frame.set_dst_zone(ori_frame().get_src_zone());
		_frame.set_src_svr_type(self_svr_type());
		_frame.set_dst_svr_type(ori_frame().get_src_svr_type());
		_frame.set_src_inst_id(self_inst_id());
		_frame.set_dst_inst_id(ori_frame().get_src_inst_id());
		_frame.set_src_trans_id(trans_id());
		_frame.set_dst_trans_id(ori_frame().get_src_trans_id());
		_frame.set_cmd(cmd() + 1);
		_frame.set_userid(userid());
		_frame.set_req_random(ori_frame().get_req_random());

		std::string data = respond.SerializePartialAsString();
		_frame.set_cmd_length(data.length());

		_buffer.Clear();
		_buffer.Write(_frame);
		_buffer.Write(data.c_str(), data.length());
		if (NetIoHandlerSgl.SendDataByFd(fd(), _buffer.Data(), _buffer.Length())) {
			return 0;
		}
		else {
			return -1;
		}
	}

private:
	AppHeadFrame _frame;
	base::Buffer _buffer;
};

REGISTER_TRANSACTION(CMD_REGISTER_SERVER_REQ, TransRegisterServer);

///////////////////////////////////////////////////////////////////////////

class TransSvrHeatBeat
	: public BaseTransaction<TransSvrHeatBeat, proto::SvrHeatBeat>{
public:
	TransSvrHeatBeat(unsigned int cmd) : BaseTransaction(cmd) {}

	int OnRequest(proto::SvrHeatBeat& request) {
		LogInfo("server heat beat: " << request.ShortDebugString());
		return 0;
	}
};

REGISTER_TRANSACTION(CMD_SVR_HEATBEAT, TransSvrHeatBeat);