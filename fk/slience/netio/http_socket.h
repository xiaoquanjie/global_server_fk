#pragma once

#include "slience/netio/config.hpp"
#include "slience/netio/http_base_socket.hpp"

M_NETIO_NAMESPACE_BEGIN

// class httpsocket
class HttpSocket :
	public HttpBaseSocket<HttpSocket, SocketLib::TcpSocket<SocketLib::IoService>, HttpSvrRecvMsg>
{
	friend class NetIo;
public:
	HttpSocket(NetIo& netio);

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket();

	HttpSvrSendMsg& GetSvrMsg();

	void SendHttpMsg();

protected:
	void Init();

	HttpSvrSendMsg _httpmsg;
};

M_NETIO_NAMESPACE_END
