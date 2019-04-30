#ifndef M_NETIO_HTTP_SOCKET_INCLUDE
#define M_NETIO_HTTP_SOCKET_INCLUDE

#include "slience/netio/config.hpp"
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
#endif