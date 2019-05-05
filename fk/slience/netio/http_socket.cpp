#include "slience/netio/netio.hpp"
#include "slience/netio/http_socket.h"

M_NETIO_NAMESPACE_BEGIN

HttpSocket::HttpSocket(NetIo& netio)
	:HttpBaseSocket(netio) {
}

SocketLib::TcpSocket<SocketLib::IoService>& HttpSocket::GetSocket() {
	return (*this->_socket);
}

HttpSvrSendMsg& HttpSocket::GetSvrMsg() {
	return _httpmsg;
}

void HttpSocket::SendHttpMsg() {
	 Send(_httpmsg._pbuffer);
	_httpmsg._pbuffer->Clear();
	_httpmsg._flag = 0;
}

void HttpSocket::Init() {
	try {
		this->_remoteep = this->_socket->RemoteEndPoint();
		this->_localep = this->_socket->LocalEndPoint();
		this->_flag = E_STATE_START;
		this->_fd = _socket->GetFd();
		shard_ptr_t<HttpSocket> ref = this->shared_from_this();
		this->_netio.OnConnected(ref);
		this->_TryRecvData();
	}
	catch (const SocketLib::SocketError& e) {
		lasterror = e;
	}
}

M_NETIO_NAMESPACE_END