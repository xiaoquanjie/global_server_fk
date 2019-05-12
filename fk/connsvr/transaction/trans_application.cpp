#include "protolib/src/svr_base.pb.h"
#include "protolib/src/cmd.pb.h"
#include "commonlib/transaction/base_transaction.h"
#include "commonlib/transaction/transaction_mgr.h"
#include "commonlib/net_handler/router_mgr.h"
#include "commonlib/net_handler/net_handler.h"
#include "connsvr/conn_svr.h"

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
				RouterMgrSgl.AddRouter(ep.Address(), ep.Port(), ptr->GetListenConnNum(), fd());
			}
		}
		return 0;
	}

	void SendRegistCmd() {
		do {
			LogInfo("try to regist server");
			proto::RegisterServerReq request;
			proto::RegisterServerRsp respond;
			request.set_server_type(ConnApplicationSgl.ServerType());
			request.set_instance_id(ConnApplicationSgl.InstanceId());
			int ret = SendMsgByFd(proto::CMD::CMD_REGISTER_SERVER_REQ, request, respond);
			if (0 == ret && respond.ret().code() == 0) {
				LogInfo("regist server success");
				break;
			}
			else {
				LogError("regist server fail: " << ret);
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
						<< " connection broken, try to reconnect");
					NetIoHandlerSgl.ConnectOne(ep.Address(),
						ep.Port(), ptr->GetListenConnType(), ptr->GetListenConnNum());
				}
			}
		}
		return 0;
	}
};

REGISTER_TRANSACTION(CMD_SOCKET_CLIENT_OUT, TransClientOut);

