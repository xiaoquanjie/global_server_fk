#ifndef M_SVR_BASE_INCLUDE
#define M_SVR_BASE_INCLUDE

#include "slience/base/buffer.hpp"
#include "slience/netio/netio.h"
#include "slience/base/timer.hpp"
#include "slience/base/util.h"
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include <sstream>

#ifndef GETSETVAR
#define GETSETVAR(type, name) \
public: \
    type get_##name() const { return this->name; } \
    void set_##name(const type& newval) { this->name = newval; } \
private: \
    type name;
#endif

#ifdef M_PLATFORM_WIN
#pragma pack(1)
struct AppHeadFrame {
#else
struct __attribute__((__packed__)) AppHeadFrame {
#endif
	GETSETVAR(base::s_uint16_t, is_broadcast);			// 消息是否广播
	GETSETVAR(base::s_uint32_t, src_svr_type);			// 源服务器类型
	GETSETVAR(base::s_uint32_t, dst_svr_type);			// 目标服务器类型
	GETSETVAR(base::s_uint32_t, src_inst_id);			// 源服务器实例
	GETSETVAR(base::s_uint32_t, dst_inst_id);			// 目标服务器实例
	GETSETVAR(base::s_uint32_t, src_trans_id);			// 源事务id
	GETSETVAR(base::s_uint32_t, dst_trans_id);			// 目标事务id
	GETSETVAR(base::s_uint32_t, cmd);					// cmd
	GETSETVAR(base::s_uint32_t, cmd_length);			// cmd长度
	GETSETVAR(base::s_uint64_t, userid);				// user id
	GETSETVAR(base::s_uint32_t, req_random);			// cmd长度

public:
	AppHeadFrame() {
		is_broadcast = 0;
		src_svr_type = 0;
		dst_svr_type = 0;
		src_inst_id = 0;
		dst_inst_id = 0;
		src_trans_id = 0;
		dst_trans_id = 0;
		cmd = 0;
		cmd_length = 0;
		userid = 0;
		req_random = 0;
	}

	std::string ToString() const {
		std::ostringstream oss;
		oss << "userid:" << userid;
		oss << " cmd:" << cmd;
		oss << " cmd_length:" << cmd_length;
		oss << " is_broadcast:" << is_broadcast;
		oss << " src_svr_type:" << src_svr_type;
		oss << " dst_svr_type:" << dst_svr_type;
		oss << " src_inst_id:" << src_inst_id;
		oss << " dst_inst_id:" << dst_inst_id;
		oss << " src_trans_id:" << src_trans_id;
		oss << " dst_trans_id:" << dst_trans_id;
		oss << " req_random:" << req_random;
		return oss.str();
	}

	void n2h() {
		is_broadcast = ntohs(is_broadcast);
		src_svr_type = ntohl(src_svr_type);
		dst_svr_type = ntohl(dst_svr_type);
		src_inst_id = ntohl(src_inst_id);
		dst_inst_id = ntohl(dst_inst_id);
		src_trans_id = ntohl(src_trans_id);
		dst_trans_id = ntohl(dst_trans_id);
		cmd = ntohl(cmd);
		cmd_length = ntohl(cmd_length);
		userid = base::ntohll(userid);
		req_random = ntohl(req_random);
	}

	void h2n() {
		is_broadcast = htons(is_broadcast);
		src_svr_type = htonl(src_svr_type);
		dst_svr_type = htonl(dst_svr_type);
		src_inst_id = htonl(src_inst_id);
		dst_inst_id = htonl(dst_inst_id);
		src_trans_id = htonl(src_trans_id);
		dst_trans_id = htonl(dst_trans_id);
		cmd = htonl(cmd);
		cmd_length = htonl(cmd_length);
		userid = base::htonll(userid);
		req_random = htonl(req_random);
	}
};
#ifdef M_PLATFORM_WIN
#pragma pack()
#endif

#ifndef M_SOCKET_IN
#define M_SOCKET_IN  (1)
#endif

#ifndef M_SOCKET_OUT
#define M_SOCKET_OUT (2)
#endif

#ifndef M_SOCKET_DATA
#define M_SOCKET_DATA (3)
#endif

#ifndef M_TCP_CONNECTOR_FD_FLAG
#define M_TCP_CONNECTOR_FD_FLAG ((base::s_int64_t)1 << 33)
#endif

#ifndef M_TCP_FD_FLAG
#define M_TCP_FD_FLAG ((base::s_int64_t)1 << 34)
#endif

#ifndef M_CHECK_IS_TCP_CONNECTOR_FD
#define M_CHECK_IS_TCP_CONNECTOR_FD(fd) (M_TCP_CONNECTOR_FD_FLAG & fd)
#endif

#ifndef M_CHECK_IS_TCP_FD
#define M_CHECK_IS_TCP_FD(fd) (M_TCP_FD_FLAG & fd)
#endif

#ifndef M_GET_TCP_FD
#define M_GET_TCP_FD(fd) (int)(fd & ~M_TCP_FD_FLAG)
#endif

#ifndef M_GET_TCP_CONNECTOR_FD
#define M_GET_TCP_CONNECTOR_FD(fd) (int)(fd & ~M_TCP_CONNECTOR_FD_FLAG)
#endif


// context
struct TcpSocketContext {
	base::s_int64_t fd;
	int msgcount;
	time_t tt;
	netiolib::TcpSocketPtr ptr;
};

struct TcpConnectorContext {
	base::s_int64_t fd;
	int msgcount;
	time_t tt;
	netiolib::TcpConnectorPtr ptr;
};

// message
struct TcpSocketMsg {
	netiolib::TcpSocketPtr ptr;
	base::Buffer buf;
	base::s_uint16_t type;
	void Clear() {
		ptr.reset();
		buf.Clear();
		type = 0;
	}
};

struct TcpConnectorMsg {
	netiolib::TcpConnectorPtr ptr;
	SocketLib::SocketError error;
	base::Buffer buf;
	base::s_uint16_t type;
	void Clear() {
		ptr.reset();
		buf.Clear();
		type = 0;
		error.Clear();
	}
};


struct tag_socket_context_fd{};
struct tag_socket_context_active{};

namespace bmi = boost::multi_index;

// multi index container
typedef bmi::multi_index_container<TcpSocketContext,
			bmi::indexed_by<
				bmi::ordered_unique<bmi::tag<tag_socket_context_fd>, bmi::member<TcpSocketContext, base::s_int64_t, &TcpSocketContext::fd> >,
				bmi::ordered_non_unique<bmi::tag<tag_socket_context_active>, bmi::member<TcpSocketContext, time_t, &TcpSocketContext::tt> >
			>
> TcpSocketContextContainer;

typedef bmi::multi_index_container<TcpConnectorContext,
	bmi::indexed_by<
	bmi::ordered_unique<bmi::tag<tag_socket_context_fd>, bmi::member<TcpConnectorContext, base::s_int64_t, &TcpConnectorContext::fd> >,
	bmi::ordered_non_unique<bmi::tag<tag_socket_context_active>, bmi::member<TcpConnectorContext, time_t, &TcpConnectorContext::tt> >
	>
> TcpConnectorContextContainer;

// modify class
class FuncModifySocketContext {
public:
	FuncModifySocketContext(int cnt, time_t tt) {
		_cnt = cnt;
		_tt = tt;
	}

	void operator()(TcpSocketContext& ctxt) {
		ctxt.msgcount = _cnt;
		ctxt.tt = _tt;
	}

	void operator()(TcpConnectorContext& ctxt) {
		ctxt.msgcount = _cnt;
		ctxt.tt = _tt;
	}

private:
	int _cnt;
	time_t _tt;
};

#endif
