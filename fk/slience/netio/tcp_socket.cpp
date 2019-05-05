#include "slience/netio/tcp_socket.h"

M_NETIO_NAMESPACE_BEGIN

TcpSocket::TcpSocket(NetIo& netio)
	:TcpStreamSocket(netio) {
}

SocketLib::TcpSocket<SocketLib::IoService>& TcpSocket::GetSocket() {
	return (*this->_socket);
}

void TcpSocket::Init() {
	try {
		_remoteep = _socket->RemoteEndPoint();
		_localep = _socket->LocalEndPoint();
		_flag = E_STATE_START;
		_fd = _socket->GetFd();
		shard_ptr_t<TcpSocket> ref = this->shared_from_this();
		_netio.OnConnected(ref);
		this->_TryRecvData();
	}
	catch (const SocketLib::SocketError& e) {
		lasterror = e;
	}
}

M_NETIO_NAMESPACE_END