#include "slience/socket/address.hpp"

M_SOCKET_NAMESPACE_BEGIN

bool AddressV4::IsV4() {
	return true;
}

bool AddressV4::IsV6()
{
	return false;
}

std::string AddressV4::Address() const {
	if (_addr._addr != 0) {
		char addr_ary[128] = { 0 };
		g_inet_ntop(M_AF_INET, (void*)&_addr, addr_ary, sizeof(addr_ary));
		return std::string(addr_ary);
	}
	return std::string("");
}

in4_addr_type AddressV4::ToBytes() const {
	return _addr;
}

s_uint32_t AddressV4::ToULong() const {
	return g_ntohl(_addr._addr);
}

bool AddressV4::IsLoopback() const {
	return (ToULong() & 0xFF000000) == 0x7F000000;
}

bool AddressV4::IsUnspecified() const {
	return (ToULong() == 0);
}

bool AddressV4::IsClassA() const {
	return (ToULong() & 0x80000000) == 0;
}

/// Determine whether the address is a class B address.
bool AddressV4::IsClassB() const {
	return (ToULong() & 0xC0000000) == 0x80000000;
}

/// Determine whether the address is a class C address.
bool AddressV4::IsClassC() const {
	return (ToULong() & 0xE0000000) == 0xC0000000;
}

/// Determine whether the address is a multicast address.
bool AddressV4::IsMulticast() const {
	return (ToULong() & 0xF0000000) == 0xE0000000;
}

AddressV4::AddressV4(in4_addr_type addr) :_addr(addr) {}

AddressV4::AddressV4(s_uint32_t addr) {
	_addr._addr = g_htonl(addr);
}
  
AddressV4::AddressV4(const char* addr) {
	g_inet_pton(M_AF_INET, addr, (void*)&_addr);
}

AddressV4::AddressV4(const std::string& addr) {
	g_inet_pton(M_AF_INET, addr.c_str(), (void*)&_addr);
}

AddressV4::AddressV4(const AddressV4& other) :_addr(other._addr) {}

AddressV4& AddressV4::operator=(const AddressV4& other) {
	_addr = other._addr;
	return *this;
}

///////////////////////////////////////////////////////////////////////////

bool AddressV6::IsV4() {
	return false;
}

bool AddressV6::IsV6() {
	return true;
}

std::string AddressV6::Address()const {
	char addr_ary[128] = { 0 };
	g_inet_ntop(M_AF_INET6, (void*)&_addr, addr_ary, sizeof(addr_ary));
	return std::string(addr_ary);
}

/// Get the address in bytes, in network byte order
in6_addr_type AddressV6::ToBytes() const {
	return _addr;
}

/// Determine whether the address is a loopback address.
bool AddressV6::IsLoopback() const {
	return ((_addr._addr.s6_addr[0] == 0) && (_addr._addr.s6_addr[1] == 0)
		&& (_addr._addr.s6_addr[2] == 0) && (_addr._addr.s6_addr[3] == 0)
		&& (_addr._addr.s6_addr[4] == 0) && (_addr._addr.s6_addr[5] == 0)
		&& (_addr._addr.s6_addr[6] == 0) && (_addr._addr.s6_addr[7] == 0)
		&& (_addr._addr.s6_addr[8] == 0) && (_addr._addr.s6_addr[9] == 0)
		&& (_addr._addr.s6_addr[10] == 0) && (_addr._addr.s6_addr[11] == 0)
		&& (_addr._addr.s6_addr[12] == 0) && (_addr._addr.s6_addr[13] == 0)
		&& (_addr._addr.s6_addr[14] == 0) && (_addr._addr.s6_addr[15] == 1));
}

/// Determine whether the address is unspecified.
bool AddressV6::IsUnspecified() const {
	return ((_addr._addr.s6_addr[0] == 0) && (_addr._addr.s6_addr[1] == 0)
		&& (_addr._addr.s6_addr[2] == 0) && (_addr._addr.s6_addr[3] == 0)
		&& (_addr._addr.s6_addr[4] == 0) && (_addr._addr.s6_addr[5] == 0)
		&& (_addr._addr.s6_addr[6] == 0) && (_addr._addr.s6_addr[7] == 0)
		&& (_addr._addr.s6_addr[8] == 0) && (_addr._addr.s6_addr[9] == 0)
		&& (_addr._addr.s6_addr[10] == 0) && (_addr._addr.s6_addr[11] == 0)
		&& (_addr._addr.s6_addr[12] == 0) && (_addr._addr.s6_addr[13] == 0)
		&& (_addr._addr.s6_addr[14] == 0) && (_addr._addr.s6_addr[15] == 0));
}

/// Determine whether the address is a mapped IPv4 address.
bool AddressV6::IsV4Mapped() const {
	return ((_addr._addr.s6_addr[0] == 0) && (_addr._addr.s6_addr[1] == 0)
		&& (_addr._addr.s6_addr[2] == 0) && (_addr._addr.s6_addr[3] == 0)
		&& (_addr._addr.s6_addr[4] == 0) && (_addr._addr.s6_addr[5] == 0)
		&& (_addr._addr.s6_addr[6] == 0) && (_addr._addr.s6_addr[7] == 0)
		&& (_addr._addr.s6_addr[8] == 0) && (_addr._addr.s6_addr[9] == 0)
		&& (_addr._addr.s6_addr[10] == 0xff) && (_addr._addr.s6_addr[11] == 0xff));
}

/// Determine whether the address is a multicast address.
bool AddressV6::IsMulticast() const {
	return (_addr._addr.s6_addr[0] == 0xff);
}

/// Determine whether the address is an IPv4-compatible address.
bool AddressV6::IsV4Compatible() const {
	return ((_addr._addr.s6_addr[0] == 0) && (_addr._addr.s6_addr[1] == 0)
		&& (_addr._addr.s6_addr[2] == 0) && (_addr._addr.s6_addr[3] == 0)
		&& (_addr._addr.s6_addr[4] == 0) && (_addr._addr.s6_addr[5] == 0)
		&& (_addr._addr.s6_addr[6] == 0) && (_addr._addr.s6_addr[7] == 0)
		&& (_addr._addr.s6_addr[8] == 0) && (_addr._addr.s6_addr[9] == 0)
		&& (_addr._addr.s6_addr[10] == 0) && (_addr._addr.s6_addr[11] == 0)
		&& !((_addr._addr.s6_addr[12] == 0)
			&& (_addr._addr.s6_addr[13] == 0)
			&& (_addr._addr.s6_addr[14] == 0)
			&& ((_addr._addr.s6_addr[15] == 0) || (_addr._addr.s6_addr[15] == 1))));
}

/// Determine whether the address is a global multicast address.
bool AddressV6::IsMulticastGlobal() const {
	return ((_addr._addr.s6_addr[0] == 0xff) && ((_addr._addr.s6_addr[1] & 0x0f) == 0x0e));
}

/// Determine whether the address is a link-local multicast address.
bool AddressV6::IsMulticastLinkLocal() const {
	return ((_addr._addr.s6_addr[0] == 0xff) && ((_addr._addr.s6_addr[1] & 0x0f) == 0x02));
}

/// Determine whether the address is a node-local multicast address.
bool AddressV6::IsMulticastNodeLocal() const {
	return ((_addr._addr.s6_addr[0] == 0xff) && ((_addr._addr.s6_addr[1] & 0x0f) == 0x01));
}

/// Determine whether the address is a org-local multicast address.
bool AddressV6::IsMulticastOrgLocal() const {
	return ((_addr._addr.s6_addr[0] == 0xff) && ((_addr._addr.s6_addr[1] & 0x0f) == 0x08));
}

/// Determine whether the address is a site-local multicast address.
bool AddressV6::IsMulticastSiteLocal() const {
	return ((_addr._addr.s6_addr[0] == 0xff) && ((_addr._addr.s6_addr[1] & 0x0f) == 0x05));
}

AddressV6::AddressV6(in6_addr_type addr) :_addr(addr) {}

AddressV6::AddressV6(const char* addr) {
	g_inet_pton(M_AF_INET6, addr, (void*)&_addr);
}

AddressV6::AddressV6(const std::string& addr) {
	g_inet_pton(M_AF_INET6, addr.c_str(), (void*)&_addr);
}

AddressV6::AddressV6(const AddressV6& other) :_addr(other._addr) {}

AddressV6& AddressV6::operator=(const AddressV6& other){
	_addr = other._addr;
	return *this;
}

M_SOCKET_NAMESPACE_END