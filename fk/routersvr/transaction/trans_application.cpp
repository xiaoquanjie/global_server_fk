#include "protolib/src/svr_base.pb.h"
#include "protolib/src/cmd.pb.h"
#include "commonlib/transaction/base_transaction.h"
#include "commonlib/transaction/transaction_mgr.h"
#include "routersvr/server_instance_mgr.h"
#include "commonlib/net_handler/net_handler.h"

class TransClientIn 
	: public BaseTransaction<TransClientIn, proto::SocketClientIn> {
public:
	TransClientIn(unsigned int cmd) : BaseTransaction(cmd) {}

	int OnRequest(proto::SocketClientIn& request) {
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
		SeverInstanceMgrSgl.DelInstance(fd());
		return 0;
	}
};

REGISTER_TRANSACTION(CMD_SOCKET_CLIENT_OUT, TransClientOut);

///////////////////////////////////////////////////////////////////////////

class TransRegisterServer 
	: public BaseTransaction<TransRegisterServer, proto::RegisterServerReq> {
public:
	TransRegisterServer(unsigned int cmd) : BaseTransaction(cmd) {}

	int OnRequest(proto::RegisterServerReq& request) {
		int ret_code = SeverInstanceMgrSgl.AddInstance(request.server_type(), request.instance_id(), fd());

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