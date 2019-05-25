#include "commonlib/net_handler/base_net_handler.h"
#include "slience/base/logger.hpp"
#include "commonlib/svr_base/svralloc.h"
#include "protolib/src/svr_base.pb.h"

const base::timestamp& BaseNetIoHandler::GetNow() const {
	return *_now;
}

void BaseNetIoHandler::SetTime(base::timestamp* now) {
	_now = now;
}

void BaseNetIoHandler::PrintStatus() {
	static base::timestamp last_print_time;
	if (GetNow().second() - last_print_time.second() >= 60) {
		LogInfo("TcpSocketMsgAlloc status----> AllocCnt=" << TcpSocketMsgAlloc::GetCount());
		LogInfo("TcpSocketMsgAlloc status----> UsingCnt=" << TcpSocketMsgAlloc::GetUsingCount());
		LogInfo("TcpConnectorMsgAlloc status----> AllocCnt=" << TcpConnectorMsgAlloc::GetCount());
		LogInfo("TcpConnectorMsgAlloc status----> UsingCnt=" << TcpConnectorMsgAlloc::GetUsingCount());
		last_print_time = GetNow();
	}
}

void BaseNetIoHandler::OnConnected(netiolib::TcpSocketPtr& clisock) {
	base::ScopedLock scoped(_tcp_socket_lock);
	TcpSocketMsg* msg = TcpSocketMsgAlloc::Alloc();
	msg->ptr = clisock;
	msg->type = M_SOCKET_IN;
	_tcp_socket_msg_list.push_back(msg);
}

void BaseNetIoHandler::OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
	base::ScopedLock scoped(_tcp_connector_lock);
	TcpConnectorMsg* msg = TcpConnectorMsgAlloc::Alloc();
	msg->error = error;
	msg->ptr = clisock;
	msg->type = M_SOCKET_IN;
	_tcp_connector_msg_list.push_back(msg);
}

void BaseNetIoHandler::OnDisconnected(netiolib::TcpSocketPtr& clisock) {
	base::ScopedLock scoped(_tcp_socket_lock);
	TcpSocketMsg* msg = TcpSocketMsgAlloc::Alloc();
	msg->ptr = clisock;
	msg->type = M_SOCKET_OUT;
	_tcp_socket_msg_list.push_back(msg);
}

void BaseNetIoHandler::OnDisconnected(netiolib::TcpConnectorPtr& clisock) {
	base::ScopedLock scoped(_tcp_connector_lock);
	TcpConnectorMsg* msg = TcpConnectorMsgAlloc::Alloc();
	msg->ptr = clisock;
	msg->type = M_SOCKET_OUT;
	_tcp_connector_msg_list.push_back(msg);
}

void BaseNetIoHandler::OnReceiveData(netiolib::TcpSocketPtr& clisock, const base::s_byte_t* data, base::s_uint32_t len) {
	base::ScopedLock scoped(_tcp_socket_lock);
	if (_tcp_socket_msg_list.size() >= M_MAX_MESSAGE_LIST) {
		// message list is too many
		LogError("tcp_socket_msg_list is too many, new message will be dropped");
		return;
	}

	TcpSocketMsg* msg = TcpSocketMsgAlloc::Alloc();
	msg->ptr = clisock;
	msg->type = M_SOCKET_DATA;
	msg->buf.Write(data, len);
	_tcp_socket_msg_list.push_back(msg);
}

void BaseNetIoHandler::OnReceiveData(netiolib::TcpConnectorPtr& clisock, const base::s_byte_t* data, base::s_uint32_t len) {
	base::ScopedLock scoped(_tcp_connector_lock);
	if (_tcp_connector_msg_list.size() >= M_MAX_MESSAGE_LIST) {
		// message list is too many
		LogError("tcp_connector_msg_list is too many, new message will be dropped");
		return;
	}

	TcpConnectorMsg* msg = TcpConnectorMsgAlloc::Alloc();
	msg->ptr = clisock;
	msg->type = M_SOCKET_DATA;
	msg->buf.Write(data, len);
	_tcp_connector_msg_list.push_back(msg);
}
