#pragma once

#include "commonlib/svr_base/svrbase.h"

// 最大消息队列
#ifndef M_MAX_MESSAGE_LIST
#define M_MAX_MESSAGE_LIST (5000)
#endif

class BaseNetIoHandler : public netiolib::NetIo {
public:
	const base::timestamp& GetNow()const;

protected:
	void SetTime(base::timestamp* now);

	void PrintStatus();

	void OnConnected(netiolib::TcpSocketPtr& clisock) override;

	void OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) override;

	void OnDisconnected(netiolib::TcpSocketPtr& clisock) override;

	void OnDisconnected(netiolib::TcpConnectorPtr& clisock) override;

	void OnReceiveData(netiolib::TcpSocketPtr& clisock, const base::s_byte_t* data, base::s_uint32_t len) override;

	void OnReceiveData(netiolib::TcpConnectorPtr& clisock, const base::s_byte_t* data, base::s_uint32_t len) override;

	// 空实现
	void OnConnected(netiolib::HttpSocketPtr& clisock) override {}
	void OnConnected(netiolib::HttpConnectorPtr& clisock, SocketLib::SocketError error) override {}
	void OnDisconnected(netiolib::HttpSocketPtr& clisock) override {}
	void OnDisconnected(netiolib::HttpConnectorPtr& clisock) override {}
	void OnReceiveData(netiolib::HttpSocketPtr& clisock, netiolib::HttpSvrRecvMsg& httpmsg) override {}
	void OnReceiveData(netiolib::HttpConnectorPtr& clisock, netiolib::HttpCliRecvMsg& httpmsg) override {}

protected:
	base::timestamp* _now;

	// message list lock
	base::MutexLock _tcp_socket_lock;
	base::MutexLock _tcp_connector_lock;
	// message list
	std::vector<TcpSocketMsg*> _tcp_socket_msg_list;
	std::vector<TcpConnectorMsg*> _tcp_connector_msg_list;
};