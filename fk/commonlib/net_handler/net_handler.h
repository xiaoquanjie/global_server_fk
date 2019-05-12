#ifndef M_NET_HANDLER_INCLUDE
#define M_NET_HANDLER_INCLUDE

#include "slience/base/singletion.hpp"
#include "commonlib/svr_base/svrbase.h"

// 最大消息队列
#ifndef M_MAX_MESSAGE_LIST
#define M_MAX_MESSAGE_LIST (5000)
#endif

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
};

class NetIoHandler : public netiolib::NetIo {
public:
	typedef m_function_t<int(base::s_int64_t fd, const AppHeadFrame& frame, 
		const char* data, base::s_uint32_t data_len)> callback_type;

	int Init(base::timestamp& now, callback_type callback);

	int Update();

	void OnTick();

	const base::timestamp& GetNow()const;

	void CheckTcpSocketExpire();

	bool SendDataByFd(base::s_int64_t fd, const char* data, base::s_int32_t len);

	void CloseFd(base::s_int64_t fd);

	netiolib::TcpConnectorPtr GetConnectorPtr(base::s_int64_t fd);

	netiolib::TcpSocketPtr GetSocketPtr(base::s_int64_t fd);

protected:
	virtual void OnConnection(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error);

	virtual void OnConnection(netiolib::TcpSocketPtr& clisock);

	virtual void OnDisConnection(netiolib::TcpConnectorPtr& clisock);

	virtual void OnDisConnection(netiolib::TcpSocketPtr& clisock);

protected:
	void OnConnected(netiolib::TcpSocketPtr& clisock) override;

	void OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) override;

	void OnDisconnected(netiolib::TcpSocketPtr& clisock) override;

	void OnDisconnected(netiolib::TcpConnectorPtr& clisock) override;

	void OnReceiveData(netiolib::TcpSocketPtr& clisock, const base::s_byte_t* data, base::s_uint32_t len) override;

	void OnReceiveData(netiolib::TcpConnectorPtr& clisock, const base::s_byte_t* data, base::s_uint32_t len) override;

protected:
	// 空实现
	void OnConnected(netiolib::HttpSocketPtr& clisock) override {}
	void OnConnected(netiolib::HttpConnectorPtr& clisock, SocketLib::SocketError error) override {}
	void OnDisconnected(netiolib::HttpSocketPtr& clisock) override {}
	void OnDisconnected(netiolib::HttpConnectorPtr& clisock) override {}
	void OnReceiveData(netiolib::HttpSocketPtr& clisock, netiolib::HttpSvrRecvMsg& httpmsg) override {}
	void OnReceiveData(netiolib::HttpConnectorPtr& clisock, netiolib::HttpCliRecvMsg& httpmsg) override {}

	TcpSocketMsg* CreateTcpSocketMsg();
	TcpConnectorMsg* CreateTcpConnectorMsg();

protected:
	size_t _msg_cache_size;
	base::timestamp* _now;

	// callback
	callback_type _callback;

	// send buff
	base::Buffer _snd_buff;

	// message list
	base::MutexLock _msg_lock;
	base::svector<TcpSocketMsg*> _tcp_socket_msg_list;
	base::svector<TcpSocketMsg*> _tcp_socket_msg_list2;
	base::svector<TcpConnectorMsg*> _tcp_connector_msg_list;
	base::svector<TcpConnectorMsg*> _tcp_connector_msg_list2;

	// socket container
	TcpSocketContextContainer _tcp_socket_container;
	TcpConnectorContextContainer _tcp_connector_container;

	// last check expire time
	base::timestamp _last_check_time;
};

#ifndef NetIoHandlerSgl
#define NetIoHandlerSgl base::singleton<NetIoHandler>::mutable_instance()
#endif

#endif