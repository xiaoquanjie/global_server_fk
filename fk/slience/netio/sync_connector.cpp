#include "slience/netio/netio.hpp"
#include "slience/netio/sync_connector.h"

M_NETIO_NAMESPACE_BEGIN

SyncConnector::SyncConnector() {
	_socket = new SocketLib::TcpConnector<SocketLib::IoService>(_ioservice);
	_flag = E_STATE_STOP;
	_readbuf = (base::s_byte_t*)g_malloc(M_SOCKET_READ_SIZE);
	g_memset(_readbuf, 0, M_SOCKET_READ_SIZE);
	_readsize = 0;
	g_memset(&_curheader, 0, sizeof(_curheader));
}

SyncConnector::~SyncConnector() {
	g_free(_readbuf);
}

bool SyncConnector::Connect(const SocketLib::Tcp::EndPoint& ep, base::s_uint32_t timeo_sec) {
	try {
		this->_socket->Connect(ep, timeo_sec);
		_flag = E_STATE_START;
		this->_remoteep = this->_socket->RemoteEndPoint();
		this->_localep = this->_socket->LocalEndPoint();
		return true;
	}
	catch (SocketLib::SocketError&) {
		return false;
	}
}

bool SyncConnector::Connect(const std::string& addr, base::s_uint16_t port, base::s_uint32_t timeo_sec) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return Connect(ep, timeo_sec);
}

const SocketLib::Tcp::EndPoint& SyncConnector::LocalEndpoint()const {
	return _localep;
}

const SocketLib::Tcp::EndPoint& SyncConnector::RemoteEndpoint()const {
	return _remoteep;
}

// 调用这个函数不意味着连接立即断开，会等所有的未处理的数据处理完就会断开
void SyncConnector::Close() {
	SocketLib::SocketError error;
	_socket->Close(error);
	_flag = E_STATE_STOP;
}

bool SyncConnector::Send(const base::s_byte_t* data, base::s_uint32_t len) {
	if (_flag != E_STATE_START)
		return  false;
	base::s_uint32_t hdrlen = (base::s_uint32_t)sizeof(PacketHeader);
	if (len > 0 && len <= (0xFFFF - hdrlen)) {
		_sndbuffer.Clear();
		PacketHeader hdr;
		hdr.size = len;
		hdr.timestamp = (unsigned int)time(0);
		hdr.h2n();
		_sndbuffer.Write(hdr);
		_sndbuffer.Write((void*)data, len);
		SocketLib::SocketError error;
		do
		{
			base::s_uint32_t snd_cnt = _socket->SendSome(_sndbuffer.Data(), _sndbuffer.Length(), error);
			if (error) {
				Close();
				return false;
			}
			_sndbuffer.CutData(snd_cnt);
			if (_sndbuffer.Length() == 0)
				break;
		} while (true);
		return true;
	}
	return false;
}

bool SyncConnector::IsConnected()const {
	return (_flag == E_STATE_START);
}

SocketLib::Buffer* SyncConnector::Recv() {
	if (_flag != E_STATE_START)
		return 0;
	_rcvbuffer.Clear();
	SocketLib::Buffer* reply = 0;
	if (_readsize>0)
		reply = _CutMsgPack(_readbuf, _readsize);
	if (!reply) {
		SocketLib::SocketError error;
		do {
			_readsize = _socket->RecvSome(_readbuf, M_SOCKET_READ_SIZE, error);
			if (_readsize == 0 || error) {
				Close();
				return 0;
			}
			reply = _CutMsgPack(_readbuf, _readsize);
			if (reply)
				break;
		} while (true);
	}
	return reply;
}

void SyncConnector::SetTimeOut(base::s_uint32_t timeo) {
	try {
		SocketLib::Opts::RcvTimeOut rtimeo(timeo, 0);
		SocketLib::Opts::SndTimeOut stimeo(timeo, 0);
		_socket->SetOption(rtimeo);
		_socket->SetOption(stimeo);
	}
	catch (...) {
	}
}

base::s_uint32_t SyncConnector::_LocalEndian()const {
	static base::s_uint32_t endian = SocketLib::detail::Util::LocalEndian();
	return endian;
}

SocketLib::Buffer* SyncConnector::_CutMsgPack(base::s_byte_t* buf, base::s_uint32_t& tran_byte) {
	// 减少内存拷贝是此函数的设计关键
	base::s_uint32_t hdrlen = (base::s_uint32_t)sizeof(PacketHeader);
	do
	{
		// 算出头部长度
		base::s_uint32_t datalen = _rcvbuffer.Length();
		if (_curheader.size == 0) {
			if (tran_byte + datalen < hdrlen) {
				_rcvbuffer.Write(buf, tran_byte);
				return 0;
			}
			else if (datalen >= hdrlen) {
				_rcvbuffer.Read(_curheader);
			}
			else {
				_rcvbuffer.Write(buf, hdrlen - datalen);
				buf += (hdrlen - datalen);
				tran_byte -= (hdrlen - datalen);
				_rcvbuffer.Read(_curheader);
			}

			// convert byte order
			_curheader.n2h();

			// check
			if (_curheader.size > (0xFFFF - hdrlen))
				return 0;
		}

		// copy body data
		datalen = _rcvbuffer.Length();
		if (tran_byte + datalen < _curheader.size) {
			_rcvbuffer.Write(buf, tran_byte);
			return 0;
		}
		else {
			_rcvbuffer.Write(buf, _curheader.size - datalen);
			buf += (_curheader.size - datalen);
			tran_byte -= (_curheader.size - datalen);

			// swap
			_curheader.size = 0;
			return &_rcvbuffer;
		}
	} while (true);
	return 0;
}


M_NETIO_NAMESPACE_END