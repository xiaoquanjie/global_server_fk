#pragma once

#include "slience/netio/config.hpp"

M_NETIO_NAMESPACE_BEGIN

template<typename T, typename SocketType>
class TcpBaseSocket : public enable_shared_from_this_t<T>
{
protected:
	struct _writerinfo_ {
		enum {
			E_MAX_BUFFER_SIZE = 1024 * 1024 * 1,
		};
		_writerinfo_();

		SocketLib::MutexLock lock;
		bool writing;
		SocketLib::Buffer msgbuffer1;
		SocketLib::Buffer msgbuffer2;
	};

public:
	TcpBaseSocket(NetIo& netio);

	virtual ~TcpBaseSocket();

	const SocketLib::Tcp::EndPoint& LocalEndpoint()const;

	const SocketLib::Tcp::EndPoint& RemoteEndpoint()const;

	int GetFd();

	// 调用这个函数不意味着连接立即断开，会等所有的未处理的数据处理完就会断开
	void Close();

	bool Send(const SocketLib::Buffer* buffer);

	bool Send(const base::s_byte_t* data, base::s_uint32_t len);

	bool IsConnected()const;

	void SetExtData(void* data, void(*func)(void*data));

	void SetExtData(void* data);

	void* GetExtData();

	void SetKeepAlive(base::s_uint32_t timeo);

protected:
	bool _CheckCanSend(int len);

	void _WriteHandler(base::s_uint32_t tran_byte, SocketLib::SocketError error);

	inline void _CloseHandler();

	void _PostClose();

	void _Close();

	bool _TrySendData();

protected:
	NetIo& _netio;
	SocketType*  _socket;
	_writerinfo_ _writer;

	// endpoint
	SocketLib::Tcp::EndPoint _localep;
	SocketLib::Tcp::EndPoint _remoteep;

	// 状态标志
	unsigned short _flag;
	void* _extdata;
	void(*_extdata_func)(void*data);

	// file descriptor, 在对象生命周期中不会被清除，不像SocketType中的fd
	int _fd;
};

template<typename T, typename SocketType>
TcpBaseSocket<T, SocketType>::_writerinfo_::_writerinfo_() {
	writing = false;
}

template<typename T, typename SocketType>
TcpBaseSocket<T, SocketType>::TcpBaseSocket(NetIo& netio)
	:_netio(netio) {
	_fd = M_INVALID_SOCKET;
	_flag = E_STATE_STOP;
	_extdata_func = 0;
	_socket = new SocketType(_netio.GetIoService());
}

template<typename T, typename SocketType>
TcpBaseSocket<T, SocketType>::~TcpBaseSocket() {
	if (_extdata_func)
		_extdata_func(_extdata);
	delete _socket;
}

template<typename T, typename SocketType>
const SocketLib::Tcp::EndPoint& TcpBaseSocket<T, SocketType>::LocalEndpoint()const {
	return _localep;
}

template<typename T, typename SocketType>
const SocketLib::Tcp::EndPoint& TcpBaseSocket<T, SocketType>::RemoteEndpoint()const {
	return _remoteep;
}

template<typename T, typename SocketType>
int TcpBaseSocket<T, SocketType>::GetFd() {
	return _fd;
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::Close() {
	/* close被调用，并不意味着连接会马上断开，socket会等所有的数据全部
	* 发送完后才断开。
	*/
	_PostClose();
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::_PostClose() {
	SocketLib::ScopedLock scoped_w(_writer.lock);
	_Close();
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::_Close() {
	if (_flag != E_STATE_CLOSE) {
		_flag = E_STATE_STOP;
		if (!_writer.writing) {
			_flag = E_STATE_CLOSE;
			SocketLib::SocketError error;
			_socket->Close(m_bind_t(&TcpBaseSocket::_CloseHandler, this->shared_from_this()), error);
		}
	}
}

template<typename T, typename SocketType>
bool TcpBaseSocket<T, SocketType>::Send(const base::s_byte_t* data, base::s_uint32_t len) {
	SocketLib::ScopedLock scoped_w(_writer.lock);
	if (!_CheckCanSend(len)) {
		return false;
	}

	_writer.msgbuffer2.Write((void*)data, len);
	_TrySendData();
	return true;
}

template<typename T, typename SocketType>
bool TcpBaseSocket<T, SocketType>::Send(const SocketLib::Buffer* buffer) {
	return Send(buffer->Raw(), buffer->Size());
}

template<typename T, typename SocketType>
bool TcpBaseSocket<T, SocketType>::IsConnected()const {
	return (_flag == E_STATE_START);
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::SetExtData(void* data, void(*func)(void*data)) {
	if (func) {
		_extdata = data;
		_extdata_func = func;
	}
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::SetExtData(void* data) {
	_extdata = data;
	_extdata_func = [](void* data)->void {
		if (data) {
			free(data);
		}
	};
}

template<typename T, typename SocketType>
void* TcpBaseSocket<T, SocketType>::GetExtData() {
	return _extdata;
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::SetKeepAlive(base::s_uint32_t timeo) {
	try {
		typename SocketLib::Opts::Keepalive kpalive(true);
		_socket->SetOption(kpalive);
#ifdef M_TCP_KEEPCNT
		typename SocketLib::Opts::TcpKeepCnt kpcnt(3);
		_socket->SetOption(kpcnt);
#endif
#ifdef M_TCP_KEEPINTVL
		typename SocketLib::Opts::TcpKeepIntvl kpintvl(2);
		_socket->SetOption(kpintvl);
#endif
#ifdef M_TCP_KEEPIDLE
		typename SocketLib::Opts::TcpKeepIdle kpidle(timeo);
		_socket->SetOption(kpidle);
#endif
	}
	catch (...) {
	}
}

template<typename T, typename SocketType>
bool TcpBaseSocket<T, SocketType>::_CheckCanSend(int len) {
	if (len <= 0) {
		return false;
	}
	if (len > M_SOCKET_PACK_SIZE) {
		M_NETIO_LOGGER(this->_socket->GetFd() << "|The package is too big to sent in the cache, so been discarded");
		return false;
	}
	if (_flag != E_STATE_START) {
		return false;
	}
	if (_writer.msgbuffer2.Size() + len > _writerinfo_::E_MAX_BUFFER_SIZE) {
		// 堆积的太多了没有发出去
		M_NETIO_LOGGER(this->_socket->GetFd() << "|There is too much data that is not sent in the cache, so been discarded");
		return false;
	}
	return true;
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::_WriteHandler(base::s_uint32_t tran_byte, SocketLib::SocketError error) {
	/*
	*  要注意防止_writer.lock死锁的问题。
	*/
	SocketLib::ScopedLock scoped_w(_writer.lock);
	_writer.writing = false;

	if (error) {
		// 出错关闭连接
		M_NETIO_LOGGER("write handler happend error:" << M_ERROR_DESC_STR(error));
		_Close();
	}
	else if (tran_byte <= 0) {
		// 连接已经关闭
		_Close();
	}
	else {
		_writer.msgbuffer1.CutData(tran_byte);
		if (!_TrySendData() && !(_flag == E_STATE_START)) {
			// 数据发送完后，如果状态不是E_STATE_START，则需要关闭写
			_socket->Shutdown(SocketLib::E_Shutdown_WR, error);
			_Close();
		}
	}
}

template<typename T, typename SocketType>
inline void TcpBaseSocket<T, SocketType>::_CloseHandler() {
	shard_ptr_t<T> ref = this->shared_from_this();
	_netio.OnDisconnected(ref);
}

template<typename T, typename SocketType>
bool TcpBaseSocket<T, SocketType>::_TrySendData() {
	if (_writer.writing)
		return true;

	if (_writer.msgbuffer1.Length() == 0) {
		_writer.msgbuffer1.Swap(_writer.msgbuffer2);
		_writer.msgbuffer2.Clear();
	}

	if (_writer.msgbuffer1.Length() > 0) {
		SocketLib::SocketError error;
		_socket->AsyncSendSome(m_bind_t(&TcpBaseSocket::_WriteHandler, this->shared_from_this(), placeholder_1, placeholder_2)
			, _writer.msgbuffer1.Data(), _writer.msgbuffer1.Length(), error);
		if (error) {
			_Close();
		}
		else {
			_writer.writing = true;
		}
		return (!error);
	}
	return false;
}

M_NETIO_NAMESPACE_END
