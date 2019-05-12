#include "commonlib/net_handler/net_handler.h"
#include "slience/base/logger.hpp"
#include "protolib/src/cmd.pb.h"
#include "protolib/src/svr_base.pb.h"
#include <sstream>

int NetIoHandler::Init(base::timestamp& now, callback_type callback) {
	_now = &now;
	_msg_cache_size = 5000;
	_callback = callback;
	return 0;
}

int NetIoHandler::Update() {
	static base::svector<TcpSocketMsg*> tmp_tcp_socket_msg_list;
	static base::svector<TcpConnectorMsg*> tmp_tcp_connector_msg_list;

	auto &tcp_socket_fd_index = _tcp_socket_container.get<tag_socket_context_fd>();
	auto &tcp_conn_fd_index = _tcp_connector_container.get<tag_socket_context_fd>();

	// swap message
	if (_tcp_socket_msg_list.size() || _tcp_connector_msg_list.size()) {
		_msg_lock.lock();
		tmp_tcp_socket_msg_list.swap(_tcp_socket_msg_list);
		tmp_tcp_connector_msg_list.swap(_tcp_connector_msg_list);
		_msg_lock.unlock();
	}

	// process connector msglist
	for (size_t idx = 0; idx < tmp_tcp_connector_msg_list.size(); ++idx) {
		TcpConnectorMsg* pmsg = tmp_tcp_connector_msg_list[idx];
		if (pmsg->type == M_SOCKET_DATA) {
			base::s_int64_t fd = M_TCP_CONNECTOR_FD_FLAG | pmsg->ptr->GetFd();
			auto tmp_iter = tcp_conn_fd_index.find(fd);
			if (tmp_iter != tcp_conn_fd_index.end()) {
				tcp_conn_fd_index.modify(tmp_iter, FuncModifySocketContext(tmp_iter->msgcount + 1, GetNow().second()));
				AppHeadFrame& pFrame = *(AppHeadFrame*)pmsg->buf.Data();
				const char* data = (const char*)pmsg->buf.Data() + sizeof(AppHeadFrame);
				pFrame.n2h();
				LogDebug("recv msg: " << pFrame.ToString());
				_callback(fd, pFrame, data, pFrame.get_cmd_length());
			}
			else {
				LogError(fd << " fd not found in _tcp_connector_container");
			}
		}
		else if (pmsg->type == M_SOCKET_IN) {
			OnConnection(pmsg->ptr, pmsg->error);
		}
		else if (pmsg->type == M_SOCKET_OUT) {
			OnDisConnection(pmsg->ptr);
		}
		else {
			assert(0);
		}
	}
	

	// process tcp socket msg list
	for (size_t idx = 0; idx < tmp_tcp_socket_msg_list.size(); ++idx) {
		TcpSocketMsg* pmsg = tmp_tcp_socket_msg_list[idx];
		if (pmsg->type == M_SOCKET_DATA) {
			base::s_int64_t fd = M_TCP_FD_FLAG | pmsg->ptr->GetFd();
			auto tmp_iter = tcp_socket_fd_index.find(fd);
			if (tmp_iter != tcp_socket_fd_index.end()) {
				tcp_socket_fd_index.modify(tmp_iter, FuncModifySocketContext(tmp_iter->msgcount + 1, GetNow().second()));
				AppHeadFrame& pFrame = *(AppHeadFrame*)pmsg->buf.Data();
				const char* data = (const char*)pmsg->buf.Data() + sizeof(AppHeadFrame);
				pFrame.n2h();
				LogDebug("recv msg: " << pFrame.ToString());
				_callback(fd, pFrame, data, pFrame.get_cmd_length());
			}
			else {
				LogError(fd << " fd not found in _tcp_socket_container");
			}
		}
		else if (pmsg->type == M_SOCKET_IN) {
			OnConnection(pmsg->ptr);
		}
		else if (pmsg->type == M_SOCKET_OUT) {
			OnDisConnection(pmsg->ptr);
		}
		else {
			assert(0);
		}
	}
	
	if (tmp_tcp_connector_msg_list.empty() && tmp_tcp_socket_msg_list.empty()) {
		// no message to process
		return -1;
	}

	// recycle msg buffer
	_msg_lock.lock();
	// tmp_tcp_connector_msg_list
	while (tmp_tcp_connector_msg_list.size()) {
		auto pmsg = tmp_tcp_connector_msg_list.back();
		if (_tcp_connector_msg_list2.size() < _msg_cache_size) {
			_tcp_connector_msg_list2.push_back(pmsg);
		}
		else {
			delete pmsg;
		}
		tmp_tcp_connector_msg_list.pop_back();
	}

	// tmp_tcp_socket_msg_list
	while (tmp_tcp_socket_msg_list.size()) {
		auto pmsg = tmp_tcp_socket_msg_list.back();
		if (_tcp_socket_msg_list2.size() < _msg_cache_size) {
			_tcp_socket_msg_list2.push_back(pmsg);
		}
		else {
			delete pmsg;
		}
		tmp_tcp_socket_msg_list.pop_back();
	}
	_msg_lock.unlock();

	// 
	return 0;
}

void NetIoHandler::OnTick() {
	CheckTcpSocketExpire();
}

const base::timestamp& NetIoHandler::GetNow()const {
	return *_now;
}

void NetIoHandler::CheckTcpSocketExpire() {
	if (GetNow().second() - _last_check_time.second() >= M_EXPIRE_CHECK_INTERVAL) {
		// ten second
		auto &tt_index = _tcp_socket_container.get<tag_socket_context_active>();
		for (auto iter = tt_index.begin(); iter != tt_index.end(); ++iter) {
			if ((GetNow().second() - iter->tt) >= M_EXPIRE_INTERVAL
				&& iter->ptr->IsConnected()) {
				LogInfo("connection expire been closed, remote_ip: " << iter->ptr->RemoteEndpoint().Address()
					<< " fd: " << iter->fd);
				iter->ptr->Close();
			}
			else {
				break;
			}
		}
		_last_check_time = GetNow();
	}
}

bool NetIoHandler::SendDataByFd(base::s_int64_t fd, const char* data, base::s_int32_t len) {
	char* data2 = const_cast<char*>(data);
	AppHeadFrame& frame = (AppHeadFrame&)(*data2);
	LogDebug("send msg: " << frame.ToString());
	frame.h2n();
	bool ret = false;

	do {
		if (M_CHECK_IS_TCP_FD(fd)) {
			auto &fd_idx = _tcp_socket_container.get<tag_socket_context_fd>();
			auto iter = fd_idx.find(fd);
			if (iter != fd_idx.end()) {
				iter->ptr->SendPacket(data, len);
				ret = true;
			}
			else {
				int real_fd = M_GET_TCP_FD(fd);
				LogError("fd is not exist, real_fd: " << real_fd << " fd: " << fd);
			}
		}
		else if (M_CHECK_IS_TCP_CONNECTOR_FD(fd)) {
			auto &fd_idx = _tcp_connector_container.get<tag_socket_context_fd>();
			auto iter = fd_idx.find(fd);
			if (iter != fd_idx.end()) {
				iter->ptr->SendPacket(data, len);
				ret = true;
			}
			else {
				int real_fd = M_GET_TCP_CONNECTOR_FD(fd);
				LogError("fd is not exist, real_fd: " << real_fd << " fd: " << fd);
			}
		}
	} while (false);

	frame.n2h();
	return ret;
}

void NetIoHandler::CloseFd(base::s_int64_t fd) {
	if (M_CHECK_IS_TCP_FD(fd)) {
		auto &fd_idx = _tcp_socket_container.get<tag_socket_context_fd>();
		auto iter = fd_idx.find(fd);
		if (iter != fd_idx.end()) {
			iter->ptr->Close();
		}
		else {
			LogWarn(fd << " fd(tcp_socket) is not exist");
		}
	}
	else if (M_CHECK_IS_TCP_CONNECTOR_FD(fd)) {
		auto &fd_idx = _tcp_connector_container.get<tag_socket_context_fd>();
		auto iter = fd_idx.find(fd);
		if (iter != fd_idx.end()) {
			iter->ptr->Close();
		}
		else {
			LogWarn(fd << " fd(tcp_connector) is not exist");
		}
	}
}

netiolib::TcpConnectorPtr NetIoHandler::GetConnectorPtr(base::s_int64_t fd) {
	auto &fd_idx = _tcp_connector_container.get<tag_socket_context_fd>();
	auto iter = fd_idx.find(fd);
	if (iter != fd_idx.end()) {
		return iter->ptr;
	}
	return netiolib::TcpConnectorPtr();
}

netiolib::TcpSocketPtr NetIoHandler::GetSocketPtr(base::s_int64_t fd) {
	auto &fd_idx = _tcp_socket_container.get<tag_socket_context_fd>();
	auto iter = fd_idx.find(fd);
	if (iter != fd_idx.end()) {
		return iter->ptr;
	}
	return netiolib::TcpSocketPtr();
}

///////////////////////////////////////////////////////////////////////////////

void NetIoHandler::OnConnection(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
	if (!error) {
		// connect success
		base::s_int64_t fd = M_TCP_CONNECTOR_FD_FLAG | clisock->GetFd();
		TcpConnectorContext context;
		context.svr_type = -1;
		context.instid = 0;
		context.fd = fd;
		context.ptr = clisock;
		context.msgcount = 0;
		context.tt = GetNow().second();

		auto &fd_index = _tcp_connector_container.get<tag_socket_context_fd>();
		if (!fd_index.insert(context).second) {
			LogError("new connection insert fail, remote_ip: " << clisock->RemoteEndpoint().Address() << " fd: " << fd << " time: " << GetNow().format_ymd_hms());
			// close socket
			clisock->Close();
		}
		else {
			LogInfo("new connection insert success, remote_ip: " << clisock->RemoteEndpoint().Address() << " fd: " << fd << " time: " << GetNow().format_ymd_hms());
			proto::SocketClientIn client_in;
			std::string str = client_in.SerializeAsString();
			AppHeadFrame frame;
			frame.set_cmd(proto::CMD::CMD_SOCKET_CLIENT_IN);
			_callback(fd, frame, str.c_str(), str.size());
		}
	}
	else {
		// 默认是重连
		const auto& ep = clisock->RemoteEndpoint();
		LogError("ip:"
			<< ep.Address()
			<< " port:"
			<< ep.Port()
			<< " connect fail, try to reconnect");
		ConnectOne(ep.Address(), ep.Port(), clisock->GetListenConnType(), clisock->GetListenConnNum());
	}
}

void NetIoHandler::OnConnection(netiolib::TcpSocketPtr& clisock) {
	base::s_int64_t fd = M_TCP_FD_FLAG | clisock->GetFd();
	TcpSocketContext context;
	context.svr_type = -1;
	context.instid = 0;
	context.fd = fd;
	context.ptr = clisock;
	context.msgcount = 0;
	context.tt = GetNow().second();

	auto &fd_index = _tcp_socket_container.get<tag_socket_context_fd>();
	if (!fd_index.insert(context).second) {
		LogError("new connection insert fail, remote_ip: " << clisock->RemoteEndpoint().Address() << " fd: " << fd << " time: " << GetNow().format_ymd_hms());
		// close socket
		clisock->Close();
	}
	else {
		LogInfo("new connection insert success, remote_ip: " << clisock->RemoteEndpoint().Address() << " fd: " << fd << " time: " << GetNow().format_ymd_hms());

		proto::SocketClientIn client_in;
		std::string str = client_in.SerializeAsString();
		AppHeadFrame frame;
		frame.set_cmd(proto::CMD::CMD_SOCKET_CLIENT_IN);
		_callback(fd, frame, str.c_str(), str.size());
	}
}

void NetIoHandler::OnDisConnection(netiolib::TcpConnectorPtr& clisock) {
	// 只是从_tcp_connector_container容器里删除fd
	base::s_int64_t fd = M_TCP_CONNECTOR_FD_FLAG | clisock->GetFd();
	auto &fd_index = _tcp_connector_container.get<tag_socket_context_fd>();
	auto iter = fd_index.find(fd);
	if (iter == fd_index.end()) {
		LogError("fd: " << fd << " not exist, this is a big bug!!!!!!!!!!!!!!!!!");
	}
	else {
		proto::SocketClientOut client_out;
		std::string str = client_out.SerializeAsString();
		AppHeadFrame frame;
		frame.set_cmd(proto::CMD::CMD_SOCKET_CLIENT_OUT);

		LogInfo("connection broken, remote_ip: " << clisock->RemoteEndpoint().Address() << " fd: " << fd << " time: " << GetNow().format_ymd_hms());
		_callback(fd, frame, str.c_str(), str.size());
		fd_index.erase(iter);
	}
}

void NetIoHandler::OnDisConnection(netiolib::TcpSocketPtr& clisock) {
	base::s_int64_t fd = M_TCP_FD_FLAG | clisock->GetFd();
	auto &fd_index = _tcp_socket_container.get<tag_socket_context_fd>();
	auto iter = fd_index.find(fd);
	if (iter == fd_index.end()) {
		LogError("fd: " << fd << " not exist, this is a big bug!!!!!!!!!!!!!!!!!");
	}
	else {
		int instid = iter->instid;
		proto::SocketClientOut client_out;
		std::string str = client_out.SerializeAsString();
		AppHeadFrame frame;
		frame.set_dst_inst_id(instid);
		frame.set_cmd(proto::CMD::CMD_SOCKET_CLIENT_OUT);

		LogInfo("connection broken, remote_ip: " << clisock->RemoteEndpoint().Address() << " fd: " << fd << " time: " << GetNow().format_ymd_hms());
		_callback(fd, frame, str.c_str(), str.size());
		fd_index.erase(iter);
	}
}

///////////////////////////////////////////////////////////////////////////////

void NetIoHandler::OnConnected(netiolib::TcpSocketPtr& clisock) {
	base::ScopedLock scoped(_msg_lock);
	TcpSocketMsg* pMessage = CreateTcpSocketMsg();
	pMessage->ptr = clisock;
	pMessage->type = M_SOCKET_IN;
	_tcp_socket_msg_list.push_back(pMessage);
}

void NetIoHandler::OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
	base::ScopedLock scoped(_msg_lock);
	TcpConnectorMsg* pMessage = CreateTcpConnectorMsg();
	pMessage->error = error;
	pMessage->ptr = clisock;
	pMessage->type = M_SOCKET_IN;
	_tcp_connector_msg_list.push_back(pMessage);
}

void NetIoHandler::OnDisconnected(netiolib::TcpSocketPtr& clisock) {
	base::ScopedLock scoped(_msg_lock);
	TcpSocketMsg* pMessage = CreateTcpSocketMsg();
	pMessage->ptr = clisock;
	pMessage->type = M_SOCKET_OUT;
	_tcp_socket_msg_list.push_back(pMessage);
}

void NetIoHandler::OnDisconnected(netiolib::TcpConnectorPtr& clisock) {
	base::ScopedLock scoped(_msg_lock);
	TcpConnectorMsg* pMessage = CreateTcpConnectorMsg();
	pMessage->ptr = clisock;
	pMessage->type = M_SOCKET_OUT;
	_tcp_connector_msg_list.push_back(pMessage);
}

void NetIoHandler::OnReceiveData(netiolib::TcpSocketPtr& clisock, const base::s_byte_t* data, base::s_uint32_t len) {
	base::ScopedLock scoped(_msg_lock);
	if (_tcp_socket_msg_list.size() >= M_MAX_MESSAGE_LIST) {
		// message list is too many
		LogError("tcp_socket_msg_list is too many, new message will be dropped");
		return;
	}

	TcpSocketMsg* pMessage = CreateTcpSocketMsg();
	pMessage->ptr = clisock;
	pMessage->type = M_SOCKET_DATA;
	pMessage->buf.Write(data, len);
	_tcp_socket_msg_list.push_back(pMessage);
}

void NetIoHandler::OnReceiveData(netiolib::TcpConnectorPtr& clisock, const base::s_byte_t* data, base::s_uint32_t len) {
	base::ScopedLock scoped(_msg_lock);
	if (_tcp_connector_msg_list.size() >= M_MAX_MESSAGE_LIST) {
		// message list is too many
		LogError("tcp_connector_msg_list is too many, new message will be dropped");
		return;
	}

	TcpConnectorMsg* pMessage = CreateTcpConnectorMsg();
	pMessage->ptr = clisock;
	pMessage->type = M_SOCKET_DATA;
	pMessage->buf.Write(data, len);
	_tcp_connector_msg_list.push_back(pMessage);
}

TcpSocketMsg* NetIoHandler::CreateTcpSocketMsg() {
	TcpSocketMsg* pMessage = 0;
	if (_tcp_socket_msg_list2.size() > 0) {
		pMessage = _tcp_socket_msg_list2.back();
		_tcp_socket_msg_list2.pop_back();
	}
	else {
		pMessage = new TcpSocketMsg;
	}
	pMessage->Clear();
	return pMessage;

}

TcpConnectorMsg* NetIoHandler::CreateTcpConnectorMsg() {
	TcpConnectorMsg* pMessage = 0;
	if (_tcp_connector_msg_list2.size() > 0) {
		pMessage = _tcp_connector_msg_list2.back();
		_tcp_connector_msg_list2.pop_back();
	}
	else {
		pMessage = new TcpConnectorMsg;
	}
	pMessage->Clear();
	return pMessage;
}