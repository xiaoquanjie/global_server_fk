#pragma once

#include "slience/netio/tcp_base_socket.hpp"

M_NETIO_NAMESPACE_BEGIN

// for http
template<typename T, typename SocketType, typename HttpMsgType>
class HttpBaseSocket :
	public TcpBaseSocket<T, SocketType>
{
protected:
	struct _readerinfo_ {
		base::s_byte_t*  readbuf;
		HttpMsgType httpmsg;

		_readerinfo_();
		~_readerinfo_();
	};

	_readerinfo_ _reader;

	void _ReadHandler(base::s_uint32_t tran_byte, SocketLib::SocketError error);

	// 裁减出数据包，返回false意味着数据包有错
	bool _CutMsgPack(base::s_byte_t* buf, base::s_uint32_t tran_byte);

	void _TryRecvData();

public:
	HttpBaseSocket(NetIo& netio);
};

template<typename T, typename SocketType, typename HttpMsgType>
HttpBaseSocket<T, SocketType, HttpMsgType>::_readerinfo_::_readerinfo_() {
	readbuf = new base::s_byte_t[M_SOCKET_READ_SIZE];
	g_memset(readbuf, 0, M_SOCKET_READ_SIZE);
}

template<typename T, typename SocketType, typename HttpMsgType>
HttpBaseSocket<T, SocketType, HttpMsgType>::_readerinfo_::~_readerinfo_() {
	delete[]readbuf;
}

template<typename T, typename SocketType, typename HttpMsgType>
void HttpBaseSocket<T, SocketType, HttpMsgType>::_ReadHandler(base::s_uint32_t tran_byte, SocketLib::SocketError error) {
	if (error) {
		// 出错关闭连接
		M_NETIO_LOGGER("read handler happend error:" << M_ERROR_DESC_STR(error));
		this->_PostClose();
	}
	else if (tran_byte <= 0) {
		// 对方关闭写
		this->_PostClose();
	}
	else {
		if (this->_flag == E_STATE_START) {
			if (_CutMsgPack(_reader.readbuf, tran_byte)) {
				_TryRecvData();
			}
			else {
				// 数据检查出错，主动断开连接
				this->_socket->Shutdown(SocketLib::E_Shutdown_RD, error);
				this->_PostClose();
			}
		}
		else {
			this->_PostClose();
		}
	}
}

template<typename T, typename SocketType, typename HttpMsgType>
bool HttpBaseSocket<T, SocketType, HttpMsgType>::_CutMsgPack(base::s_byte_t* buf, base::s_uint32_t tran_byte) {
	shard_ptr_t<T> ref;
	while (true) {
		base::s_uint32_t copy_len = (base::s_uint32_t)_reader.httpmsg.Parse(buf, tran_byte);
		if (copy_len == 0 || copy_len <= tran_byte) {
			if (!ref)
				ref = this->shared_from_this();
			this->_netio.OnReceiveData(ref, _reader.httpmsg);
			_reader.httpmsg.Clear();
		}
		tran_byte -= copy_len;
		if (tran_byte == 0)
			break;
		buf += copy_len;
	}
	return true;
}

template<typename T, typename SocketType, typename HttpMsgType>
void HttpBaseSocket<T, SocketType, HttpMsgType>::_TryRecvData() {
	SocketLib::SocketError error;
	this->_socket->AsyncRecvSome(m_bind_t(&HttpBaseSocket::_ReadHandler, this->shared_from_this(), placeholder_1, placeholder_2)
		, _reader.readbuf, M_SOCKET_READ_SIZE, error);
	if (error)
		this->_PostClose();
}

template<typename T, typename SocketType, typename HttpMsgType>
HttpBaseSocket<T, SocketType, HttpMsgType>::HttpBaseSocket(NetIo& netio)
	:TcpBaseSocket<T, SocketType>(netio) {
}

M_NETIO_NAMESPACE_END
