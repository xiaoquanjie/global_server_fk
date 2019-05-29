#include "protolib/src/svr_base.pb.h"
#include "protolib/src/cmd.pb.h"
#include "commonlib/transaction/base_transaction.h"
#include "commonlib/transaction/transaction_mgr.h"
#include "routersvr/server_instance_mgr.h"
#include "routersvr/transfer_mgr.h"
#include "commonlib/net_handler/net_handler.h"

class TransClientIn 
	: public BaseTransaction<TransClientIn, proto::SocketClientIn> {
public:
	TransClientIn(unsigned int cmd) : BaseTransaction(cmd) {}

	int OnRequest(proto::SocketClientIn& request) {
		if (M_CHECK_IS_TCP_CONNECTOR_FD(fd())) {
			// 判断是不是连transfer的连接
			auto ptr = NetIoHandlerSgl.GetConnectorPtr(fd());
			if (!ptr) {
				return 0;
			}
			if (ptr->GetListenConnType() == Enum_ConnType_Transfer) {
				SendRegistCmd();
				const auto& ep = ptr->RemoteEndpoint();
				TransferMgrSgl.AddTransfer(ep.Address(), ep.Port(), ptr->GetListenConnNum(), fd());
			}
		}
		return 0;
	}

	void SendRegistCmd() {
		// 一直到注册成功才退出
		int retry_type = 0;
		proto::RegisterServerReq request;
		proto::RegisterServerRsp respond;
		do {
			if (retry_type == 0) {
				LogInfo("try to regist self server to transfersvr");
				request.set_server_type(self_svr_type());
				request.set_instance_id(self_inst_id());
				request.set_server_zone(self_svr_zone());
				int ret = SendMsgByFd(proto::CMD::CMD_REGISTER_SERVER_REQ, request, respond);
				if (ret != 0) {
					retry_type = -2;
					continue;
				}
				if (respond.ret().code() == 0) {
					LogInfo("regist self server  to transfersvr success");
					break;
				} else {
					retry_type = -1;
					continue;
				}
			}
			else if (retry_type == -1) {
				LogInfo("regiest self server  to transfersvr fail, because of " << respond.ShortDebugString());
				retry_type = 0;
				Wait(1000);
			}
			else {
				LogError("regist self server  to transfersvr fail, because of timeout");
				retry_type = 0;
			}
		} while (true);
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
			onTransfer();
		}
		else {
			OnRouter();
		}
		return 0;
	}

	void onTransfer () {
		auto ptr = NetIoHandlerSgl.GetConnectorPtr(fd());
		if (!ptr) {
			return;
		}

		if (ptr->GetListenConnType() == Enum_ConnType_Transfer) {
			// 重连
			const auto& ep = ptr->RemoteEndpoint();
			if (TransferMgrSgl.ExistTransfer(ep.Address(), ep.Port(), ptr->GetListenConnNum())) {
				LogError("ip:"
					<< ep.Address()
					<< " port:"
					<< ep.Port()
					<< " transfersvr's connection broken, try to reconnect");
				NetIoHandlerSgl.ConnectOne(ep.Address(),
					ep.Port(), ptr->GetListenConnType(), ptr->GetListenConnNum());
			}
		}
	}

	void OnRouter() {
		SeverInstanceMgrSgl.DelInstance(fd());
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
				LogWarn("register server failed, because of different svr_zone, self_zone="
					<< self_svr_zone()
					<< " request_zone="
					<< request.server_zone()
				);
				break;
			}

			ret_code = SeverInstanceMgrSgl.AddInstance(request.server_type(), request.instance_id(), fd());

		} while (false);

		proto::RegisterServerRsp respond;
		respond.mutable_ret()->set_code(ret_code);

		AppHeadFrame frame;
		frame.set_is_broadcast(false);
		frame.set_src_svr_type(self_svr_type());
		frame.set_dst_svr_type(ori_frame().get_src_svr_type());
		frame.set_src_inst_id(self_inst_id());
		frame.set_dst_inst_id(ori_frame().get_src_inst_id());
		frame.set_src_trans_id(trans_id());
		frame.set_dst_trans_id(ori_frame().get_src_trans_id());
		frame.set_cmd(cmd() + 1);
		frame.set_userid(userid());
		frame.set_req_random(ori_frame().get_req_random());

		std::string data = respond.SerializePartialAsString();
		frame.set_cmd_length(data.length());

		base::Buffer buffer;
		buffer.Write(frame);
		buffer.Write(data.c_str(), data.length());
		if (NetIoHandlerSgl.SendDataByFd(fd(), buffer.Data(), buffer.Length())) {
			return 0;
		}
		else {
			return -1;
		}
		return 0;
	}
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