#pragma once

#include "slience/netio/http_base_socket.hpp"

M_NETIO_NAMESPACE_BEGIN

class HttpConnector : 
	public HttpBaseSocket<HttpConnector, SocketLib::TcpConnector<SocketLib::IoService>
		,HttpCliRecvMsg> {
public:
	HttpConnector(NetIo& netio);

	SocketLib::TcpConnector<SocketLib::IoService>& GetSocket();

	bool Connect(const SocketLib::Tcp::EndPoint& ep);

	bool Connect(const std::string& addr, base::s_uint16_t port);

	void AsyncConnect(const SocketLib::Tcp::EndPoint& ep);

	void AsyncConnect(const std::string& addr, base::s_uint16_t port);

protected:
	void _ConnectHandler(const SocketLib::SocketError& error, HttpConnectorPtr conector);
};


M_NETIO_NAMESPACE_END
