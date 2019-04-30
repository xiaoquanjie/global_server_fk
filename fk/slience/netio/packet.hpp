#ifndef M_NETIO_PACKET_INCLUDE
#define M_NETIO_PACKET_INCLUDE

#include "slience/netio/config.hpp"
M_NETIO_NAMESPACE_BEGIN

#ifdef M_PLATFORM_WIN
#pragma pack(1)
struct PacketHeader {
	int	timestamp;
	unsigned int size;
	void n2h();
	void h2n();
};
#pragma pack()
#else
struct __attribute__((__packed__)) PacketHeader {
	int	timestamp;
	unsigned int size;
	void n2h();
	void h2n();
};
#endif

inline void PacketHeader::n2h() {
	timestamp = ntohl(timestamp);
	size = ntohl(size);
}

inline void PacketHeader::h2n() {
	timestamp = htonl(timestamp);
	size = htonl(size);
}

#define M_SOCKET_READ_SIZE (4*1024)
#define M_SOCKET_PACK_SIZE (400*1024)
#define M_PACGET_CODE (0xFCFCFC)

M_NETIO_NAMESPACE_END
#endif