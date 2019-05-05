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

#include "slience/socket/config.hpp"

M_SOCKET_NAMESPACE_BEGIN

struct in4_addr_type {
	s_uint32_t _addr;
};

struct in6_addr_type { 
	in6_addr _addr;
};

struct AddressV4
{
	static bool IsV4();

	static bool IsV6();

	std::string Address() const;

	/// Get the address in bytes, in network byte order
	in4_addr_type ToBytes() const;

	/// Get the address as an unsigned long in host byte order
	s_uint32_t ToULong() const;

	/// Determine whether the address is a loopback address.
	bool IsLoopback() const;

	/// Determine whether the address is unspecified.
	bool IsUnspecified() const;

	/// Determine whether the address is a class A address.
	bool IsClassA() const;

	/// Determine whether the address is a class B address.
	bool IsClassB() const;

	/// Determine whether the address is a class C address.
	bool IsClassC() const;

	/// Determine whether the address is a multicast address.
	bool IsMulticast() const;

	AddressV4(in4_addr_type addr);

	AddressV4(s_uint32_t addr);

	explicit AddressV4(const char* addr);

	AddressV4(const std::string& addr);

	AddressV4(const AddressV4& other);

	AddressV4& operator=(const AddressV4& other);

	friend bool operator==(const AddressV4& i1, const AddressV4& i2) {
		return (i1._addr._addr == i2._addr._addr);
	}

	friend bool operator!=(const AddressV4& i1, const AddressV4& i2) {
		return (i1._addr._addr != i2._addr._addr);
	}

protected:
	in4_addr_type _addr;
};

///////////////////////////////////////////////////////////////////////////

struct AddressV6
{
	static bool IsV4();

	static bool IsV6();

	std::string Address() const;

	/// Get the address in bytes, in network byte order
	in6_addr_type ToBytes() const;

	/// Determine whether the address is a loopback address.
	bool IsLoopback() const;

	/// Determine whether the address is unspecified.
	bool IsUnspecified() const;

	/// Determine whether the address is a mapped IPv4 address.
	bool IsV4Mapped() const;

	/// Determine whether the address is a multicast address.
	bool IsMulticast() const;

	/// Determine whether the address is an IPv4-compatible address.
	bool IsV4Compatible() const;

	/// Determine whether the address is a global multicast address.
	bool IsMulticastGlobal() const;

	/// Determine whether the address is a link-local multicast address.
	bool IsMulticastLinkLocal() const;

	/// Determine whether the address is a node-local multicast address.
	bool IsMulticastNodeLocal() const;

	/// Determine whether the address is a org-local multicast address.
	bool IsMulticastOrgLocal() const;

	/// Determine whether the address is a site-local multicast address.
	bool IsMulticastSiteLocal() const;

	AddressV6(in6_addr_type addr);

	explicit AddressV6(const char* addr);

	AddressV6(const std::string& addr);

	AddressV6(const AddressV6& other);

	AddressV6& operator=(const AddressV6& other);

	friend bool operator==(const AddressV6& i1, const AddressV6& i2)
	{
		return g_memcpy((void*)&i1, (void*)&i2, sizeof(in6_addr_type)) == 0;
	}

	friend bool operator!=(const AddressV6& i1, const AddressV6& i2)
	{
		return !(i1 == i2);
	}
protected:
	in6_addr_type _addr;
};


M_SOCKET_NAMESPACE_END
