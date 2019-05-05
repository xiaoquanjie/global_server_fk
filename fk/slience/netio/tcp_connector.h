#pragma once

#include "slience/netio/config.hpp"

M_NETIO_NAMESPACE_BEGIN

// class tcpconnector
class TcpConnector
	: public TcpStreamSocket<TcpConnector, SocketLib::TcpConnector<SocketLib::IoService> > {
public:
	TcpConnector(NetIo& netio);

	SocketLib::TcpConnector<SocketLib::IoService>& GetSocket();

	bool Connect(const SocketLib::Tcp::EndPoint& ep, base::s_uint32_t timeo_sec = -1);

	bool Connect(const std::string& addr, base::s_uint16_t port, base::s_uint32_t timeo_sec = -1);

	void AsyncConnect(const SocketLib::Tcp::EndPoint& ep, SocketLib::SocketError& error);

	void AsyncConnect(const std::string& addr, base::s_uint16_t port, SocketLib::SocketError& error);

protected:
	void _ConnectHandler(const SocketLib::SocketError& error, TcpConnectorPtr conector);
};

M_NETIO_NAMESPACE_END
