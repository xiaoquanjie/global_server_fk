#ifndef M_NETIO_TCP_STREAM_SOCKET_INCLUDE
#define M_NETIO_TCP_STREAM_SOCKET_INCLUDE

#include "slience/netio/config.hpp"
M_NETIO_NAMESPACE_BEGIN

template<typename T, typename SocketType>
class TcpStreamSocket : public TcpBaseSocket<T, SocketType> {
protected:
	struct _readerinfo_ {
		base::s_byte_t* readbuf;
		SocketLib::Buffer msgbuffer;
		
		_readerinfo_();
		~_readerinfo_();
	};

	_readerinfo_ _reader;

	void _ReadHandler(base::s_uint32_t tran_byte, SocketLib::SocketError error);

	// 裁减出数据包，返回false意味着数据包有错
	bool _CutMsgPack(base::s_byte_t* buf, base::s_uint32_t tran_byte);

	void _TryRecvData();

public:
	TcpStreamSocket(NetIo& netio);

	bool SendPacket(const base::s_byte_t* data, base::s_uint32_t len);

	template<typename MsgHeadType>
	bool SendPacket(const MsgHeadType& head, const base::s_byte_t* data,
		base::s_uint32_t len);
};

/////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, typename SocketType>
TcpStreamSocket<T, SocketType>::_readerinfo_::_readerinfo_() {
	readbuf = new base::s_byte_t[M_SOCKET_READ_SIZE];
	g_memset(readbuf, 0, M_SOCKET_READ_SIZE);
}

template<typename T, typename SocketType>
TcpStreamSocket<T, SocketType>::_readerinfo_::~_readerinfo_() {
	delete[]readbuf;
}

template<typename T, typename SocketType>
void TcpStreamSocket<T, SocketType>::_ReadHandler(base::s_uint32_t tran_byte, SocketLib::SocketError error) {
	do {
		// 出错关闭连接
		if (error) {
			M_NETIO_LOGGER("read handler happend error:" << M_ERROR_DESC_STR(error));
			break;
		}
		// 对方关闭写
		if (tran_byte <= 0)
			break;

		// 我方post了关闭
		if (this->_flag != E_STATE_START)
			break;

		if (_CutMsgPack(_reader.readbuf, tran_byte)) {
			_TryRecvData();
			return;
		}
		else {
			// 数据检查出错，主动断开连接
			this->_socket->Shutdown(SocketLib::E_Shutdown_RD, error);
		}
	} while (false);

	this->_PostClose();
}

template<typename T, typename SocketType>
bool TcpStreamSocket<T, SocketType>::_CutMsgPack(base::s_byte_t* buf, base::s_uint32_t tran_byte) {
	// 减少内存拷贝是此函数的设计关键
	base::s_uint32_t hdrlen = (base::s_uint32_t)sizeof(PacketHeader);
	shard_ptr_t<T> ref;
	base::s_byte_t* data = 0;
	base::s_uint32_t datalen = 0;

	do {
		if (_reader.msgbuffer.Length() == 0) {
			if (tran_byte < hdrlen) {
				_reader.msgbuffer.Write(buf, tran_byte);
				break;
			}
			PacketHeader* header = (PacketHeader*)buf;
			header->n2h();
			if (tran_byte - hdrlen < header->size) {
				header->h2n();
				_reader.msgbuffer.Write(buf, tran_byte);
				break;
			}
			if (header->timestamp != M_PACGET_CODE) {
				assert(0);
				return false;
			}
			
			data = buf + hdrlen;
			datalen = header->size;
			tran_byte -= (hdrlen + header->size);
			buf += (hdrlen + header->size);
		}
		else {
			base::s_uint32_t buf_len = _reader.msgbuffer.Length();
			if (buf_len + tran_byte < hdrlen) {
				_reader.msgbuffer.Write(buf, tran_byte);
				break;
			}
			if (buf_len < hdrlen) {
				_reader.msgbuffer.Write(buf, hdrlen - buf_len);
				buf += (hdrlen - buf_len);
				tran_byte -= (hdrlen - buf_len);
			}
			PacketHeader* header = (PacketHeader*)_reader.msgbuffer.Data();
			header->n2h();
			buf_len = _reader.msgbuffer.Length() - hdrlen;
			if (header->size > (tran_byte += buf_len)) {
				header->h2n();
				_reader.msgbuffer.Write(buf, tran_byte);
				break;
			}
			if (header->timestamp != M_PACGET_CODE) {
				assert(0);
				return false;
			}

			_reader.msgbuffer.Write(buf, (header->size - buf_len));
			tran_byte -= (header->size - buf_len);
			buf += (header->size - buf_len);
			data = _reader.msgbuffer.Data();
			datalen = header->size;
		}

		if (data) {
			if (datalen > M_SOCKET_PACK_SIZE) {
				return false;
			}
			if (!ref) {
				ref = this->shared_from_this();
			}
			this->_netio.OnReceiveData(ref, data, datalen);
			data = 0;
			datalen = 0;
			_reader.msgbuffer.Clear();
		}
	} while (true);
	return true;
}

template<typename T, typename SocketType>
void TcpStreamSocket<T, SocketType>::_TryRecvData() {
	SocketLib::SocketError error;
	this->_socket->AsyncRecvSome(m_bind_t(&TcpStreamSocket::_ReadHandler, this->shared_from_this(), placeholder_1, placeholder_2)
		, _reader.readbuf, M_SOCKET_READ_SIZE, error);
	if (error)
		this->_PostClose();
}

template<typename T, typename SocketType>
TcpStreamSocket<T, SocketType>::TcpStreamSocket(NetIo& netio)
	:TcpBaseSocket<T, SocketType>(netio) {
}

template<typename T, typename SocketType>
bool TcpStreamSocket<T, SocketType>::SendPacket(const base::s_byte_t* data,
	base::s_uint32_t len) {
	SocketLib::ScopedLock scoped_w(this->_writer.lock);
	if (!this->_CheckCanSend(len + sizeof(PacketHeader))) {
		return false;
	}

	PacketHeader hdr;
	hdr.size = len;
	hdr.timestamp = M_PACGET_CODE;
	hdr.h2n();
	this->_writer.msgbuffer2.Write(hdr);
	this->_writer.msgbuffer2.Write((void*)data, len);
	this->_TrySendData();
	return true;
}

template<typename T, typename SocketType>
template<typename MsgHeadType>
bool TcpStreamSocket<T, SocketType>::SendPacket(const MsgHeadType& head, 
	const base::s_byte_t* data, base::s_uint32_t len) {
	SocketLib::ScopedLock scoped_w(this->_writer.lock);
	if (!this->_CheckCanSend(len + sizeof(PacketHeader) + sizeof(MsgHeadType))) {
		return false;
	}

	PacketHeader hdr;
	hdr.size = len + sizeof(MsgHeadType);
	hdr.timestamp = M_PACGET_CODE;
	hdr.h2n();
	this->_writer.msgbuffer2.Write(hdr);
	this->_writer.msgbuffer2.Write(head);
	this->_writer.msgbuffer2.Write((void*)data, len);
	this->_TrySendData();
	return true;
}

M_NETIO_NAMESPACE_END
#endif