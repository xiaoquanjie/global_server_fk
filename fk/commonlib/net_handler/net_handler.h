#pragma once

#include "slience/base/singletion.hpp"
#include "commonlib/net_handler/base_net_handler.h"

// expire检查间隔
#ifndef M_EXPIRE_CHECK_INTERVAL
#define M_EXPIRE_CHECK_INTERVAL (15)
#endif

// expire时长
#ifndef M_EXPIRE_INTERVAL
#define M_EXPIRE_INTERVAL (30)
#endif

enum ConnType {
	Enum_ConnType_Router = 1,
	Enum_ConnType_Transfer = 2,
};

enum ListenType {
	Enum_ListenType_Router = 1,
	Enum_ListenType_Conn = 2,
	Enum_ListenType_Transfer = 3,
};

class NetIoHandler : public BaseNetIoHandler {
public:
	typedef m_function_t<int(base::s_int64_t fd, const AppHeadFrame& frame, 
		const char* data, base::s_uint32_t data_len)> callback_type;

	int Init(base::timestamp& now, callback_type callback);

	int Update();

	void OnTick();

	void CheckTcpSocketExpire();

	bool SendDataByFd(base::s_int64_t fd, const char* data, base::s_int32_t len);

	void CloseFd(base::s_int64_t fd);

	netiolib::TcpConnectorPtr GetConnectorPtr(base::s_int64_t fd);

	netiolib::TcpSocketPtr GetSocketPtr(base::s_int64_t fd);

protected:
	int ConsumTcpSocketMsg();

	int ConsumTcpConnectorMsg();

	virtual void OnConnection(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error);

	virtual void OnConnection(netiolib::TcpSocketPtr& clisock);

	virtual void OnDisConnection(netiolib::TcpConnectorPtr& clisock);

	virtual void OnDisConnection(netiolib::TcpSocketPtr& clisock);

protected:
	// callback
	callback_type _callback;

	// socket container
	TcpSocketContextContainer _tcp_socket_container;
	TcpConnectorContextContainer _tcp_connector_container;

	// last check expire time
	base::timestamp _last_check_time;
};

#ifndef NetIoHandlerSgl
#define NetIoHandlerSgl base::singleton<NetIoHandler>::mutable_instance()
#endif
