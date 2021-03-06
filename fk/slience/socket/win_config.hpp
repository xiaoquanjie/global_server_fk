/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/8/10
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#pragma once

#ifdef M_PLATFORM_WIN
#define M_INVALID_SOCKET	INVALID_SOCKET
#define M_SOCKET_ERROR		SOCKET_ERROR
#define M_EALREADY			WSAEALREADY			// 已调用过connect
#define M_EISCONN			WSAEISCONN			// 已连接
#define M_EWOULDBLOCK		WSAEWOULDBLOCK		// 本该阻塞
#define M_ECONNRESET		WSAECONNRESET	    // 返回前连接中止
#define M_EINPROGRESS		WSAEINPROGRESS
#define M_ETIMEDOUT			WSAETIMEDOUT		// 超时
// 链接关闭方式
#define M_SHUT_RD			SD_RECEIVE			
#define M_SHUT_WR			SD_SEND
#define M_SHUT_RDWR			SD_BOTH
#define M_ECONNABORTED		ECONNABORTED
#define M_SOCKET_T			SOCKET   
#define M_ASYNC_CON_ERR		M_EWOULDBLOCK
#define M_ERROR_IO_PENDING	ERROR_IO_PENDING

// 为了与linux版本保持一致
#define M_ESHOULDBLOCK		M_EWOULDBLOCK 
#endif // M_PLATFORM_WIN
