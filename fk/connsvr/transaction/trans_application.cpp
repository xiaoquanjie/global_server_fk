#include "protolib/src/svr_base.pb.h"
#include "protolib/src/cmd.pb.h"
#include "commonlib/transaction/base_transaction.h"
#include "commonlib/transaction/transaction_mgr.h"
#include "commonlib/net_handler/router_mgr.h"
#include "commonlib/net_handler/net_handler.h"
#include "slience/base/random.hpp"

class TransClientIn 
	: public BaseTransaction<TransClientIn, proto::SocketClientIn> {
public:
	TransClientIn(unsigned int cmd) : BaseTransaction(cmd) {}

	int OnRequest(proto::SocketClientIn& request) {
		if (M_CHECK_IS_TCP_CONNECTOR_FD(fd())) {
			netiolib::TcpConnectorPtr ptr = NetIoHandlerSgl.GetConnectorPtr(fd());
			if (!ptr) {
				return 0;
			}
			if (ptr->GetListenConnType() == Enum_ConnType_Router) {
				SendRegistCmd();
				const auto& ep = ptr->RemoteEndpoint();
				RouterMgrSgl.LoginRouter(ep.Address(), ep.Port(), ptr->GetListenConnNum(), fd());
			}
		}
		return 0;
	}

	void SendRegistCmd() {
		int retry_type = 0;
		proto::RegisterServerReq request;
		proto::RegisterServerRsp respond;
		do {
			int ret = 0;
			if (retry_type == 0) {
				LogInfo("try to regist self server to routersvr");
				request.set_server_type(self_svr_type());
				request.set_instance_id(self_inst_id());
				request.set_server_zone(self_svr_zone());
				ret = SendMsgToRouterByFd(proto::CMD::CMD_REGISTER_SERVER_REQ, request, respond);
				if (ret != 0) {
					retry_type = -2;
					continue;
				}
				if (respond.ret().code() == 0) {
					LogInfo("regist self server  to routersvr success");
					break;
				}
				else {
					retry_type = -1;
					continue;
				}
			}
			else if (retry_type == -1) {
				LogInfo("regiest self server  to routersvr fail, because of " << respond.ShortDebugString());
				retry_type = 0;
				Wait(1000);
			}
			else {
				LogError("regist self server  to routersvr fail, because of timeout: " << ret);
				retry_type = 0;
			}
		} while (true);
	}

	int SendMsgToRouterByFd(int cmd, google::protobuf::Message& request) {
		set_req_random(base::random().rand(10000, 100000));
		int ret = RouterMgrSgl.SendMsgByFd(fd(),
			cmd,
			userid(),
			false,
			self_svr_zone(),
			proto::SVR_TYPE_ROUTER,
			0,
			trans_id(),
			0,
			req_random(),
			request);
		return ret;
	}

	int SendMsgToRouterByFd(int cmd, google::protobuf::Message& request
		, google::protobuf::Message& respond) {
		if (0 != SendMsgToRouterByFd(cmd, request)) {
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
			if (ptr->GetListenConnType() == Enum_ConnType_Router) {
				const auto& ep = ptr->RemoteEndpoint();
				if (RouterMgrSgl.ExistRouter(ep.Address(), ep.Port(), ptr->GetListenConnNum())) {
					// 重连
					LogError("ip:" 
						<< ep.Address() 
						<< " port:"
						<< ep.Port()
						<< " routersvr's connection broken, try to reconnect");
					NetIoHandlerSgl.ConnectOne(ep.Address(),
						ep.Port(), ptr->GetListenConnType(), ptr->GetListenConnNum());
				}
			}
		}
		return 0;
	}
};

REGISTER_TRANSACTION(CMD_SOCKET_CLIENT_OUT, TransClientOut);

