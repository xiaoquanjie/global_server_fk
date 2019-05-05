/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/11/10
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#pragma once

#include "slience/netio/config.hpp"
#include "slience/base/thread.hpp"

M_NETIO_NAMESPACE_BEGIN

class NetIo;
class TcpSocket;
class TcpConnector;
class HttpSocket;
class HttpConnector;
class SyncConnector;

typedef SocketLib::Buffer Buffer;
typedef shard_ptr_t<SocketLib::Buffer> BufferPtr;
typedef shard_ptr_t<TcpSocket>		   TcpSocketPtr;
typedef shard_ptr_t<TcpConnector>	   TcpConnectorPtr;
typedef shard_ptr_t<HttpSocket>		   HttpSocketPtr;
typedef shard_ptr_t<HttpConnector>	   HttpConnectorPtr;
typedef shard_ptr_t<SocketLib::TcpAcceptor<SocketLib::IoService> > TcpAcceptorPtr;
typedef shard_ptr_t<SyncConnector>  SyncConnectorPtr;

#ifndef lasterror
#define lasterror base::tlsdata<SocketLib::SocketError,0>::data()
#endif

class NetIo {
public:
	NetIo();
	NetIo(base::s_uint32_t backlog);

	virtual ~NetIo();

	// 建立一个监听
	bool ListenOne(const SocketLib::Tcp::EndPoint& ep);
	bool ListenOne(const std::string& addr, base::s_uint16_t port);

	// 建立一个http监听
	bool ListenOneHttp(const SocketLib::Tcp::EndPoint& ep);
	bool ListenOneHttp(const std::string& addr, base::s_uint16_t port);

	// 异步建接
	void ConnectOne(const SocketLib::Tcp::EndPoint& ep);
	void ConnectOne(const std::string& addr, base::s_uint16_t port);

	void ConnectOneHttp(const SocketLib::Tcp::EndPoint& ep);
	void ConnectOneHttp(const std::string& addr, base::s_uint16_t port);

	virtual void Start(unsigned int thread_cnt, bool isco = false);
	virtual void Stop();
	virtual void RunHandler();
	size_t  ServiceCount();

	// 获取最后的异常
	SocketLib::SocketError GetLastError()const;
	SocketLib::IoService& GetIoService();
	base::s_uint32_t LocalEndian()const;

	/*
	*以下三个函数定义为虚函数，以便根据实际业务的模式来做具体模式的消息包分发处理。
	*保证同一个socket，以下三个函数的调用遵循OnConnected -> OnReceiveData -> OnDisconnected的顺序。
	*保证同一个socket，以下后两个函数的调用都在同一个线程中
	*/

	// 连线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnConnected(TcpSocketPtr& clisock);
	virtual void OnConnected(TcpConnectorPtr& clisock, SocketLib::SocketError error);
	virtual void OnConnected(HttpSocketPtr& clisock);
	virtual void OnConnected(HttpConnectorPtr& clisock, SocketLib::SocketError error);

	// 掉线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnDisconnected(TcpSocketPtr& clisock);
	virtual void OnDisconnected(TcpConnectorPtr& clisock);
	virtual void OnDisconnected(HttpSocketPtr& clisock);
	virtual void OnDisconnected(HttpConnectorPtr& clisock);

	// 数据包通知,这个函数里不要处理业务，防止堵塞
	virtual void OnReceiveData(TcpSocketPtr& clisock, const base::s_byte_t* data, base::s_uint32_t len);
	virtual void OnReceiveData(TcpConnectorPtr& clisock, const base::s_byte_t* data, base::s_uint32_t len);
	virtual void OnReceiveData(HttpSocketPtr& clisock, HttpSvrRecvMsg& httpmsg);
	virtual void OnReceiveData(HttpConnectorPtr& clisock, HttpCliRecvMsg& httpmsg);

protected:
	void _Start(void*p);
	void _AcceptHandler(SocketLib::SocketError error, TcpSocketPtr& clisock, 
		TcpAcceptorPtr& acceptor);
	void _AcceptHttpHandler(SocketLib::SocketError error, HttpSocketPtr& clisock, 
		TcpAcceptorPtr& acceptor);

protected:
	NetIo(const NetIo&);
	NetIo& operator=(const NetIo&);

protected:
	SocketLib::IoService   _ioservice;
	base::s_uint32_t  _backlog;
	base::s_uint32_t  _endian;
	base::slist<base::thread*> _threadlist;
};

enum {
	E_STATE_START = 1,
	E_STATE_STOP,
	E_STATE_CLOSE,
};

M_NETIO_NAMESPACE_END
