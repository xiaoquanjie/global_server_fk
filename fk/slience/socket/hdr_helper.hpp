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

#include "slience/socket/pro_hdr.hpp"

M_SOCKET_NAMESPACE_BEGIN

class HdrHelper
{
public:

	virtual ~HdrHelper() {}

	// 计算校验码，算法适用于IPV4，ICMPV4，ICMPV6，IGMPV4，UDP，TCP
	static s_uint16_t CheckSum(const s_uint16_t* pAddr, const s_int32_t aiLen);

	static s_int32_t GetIcmpTypeCnt();

	static const icmp_type_t* GetIcmpType(s_int32_t aiIdx);
};

class IpHdrHelper : public HdrHelper
{
public:

	IpHdrHelper(void* pBuf);

	// 获取版本号
	s_uint32_t GetVersion() const;

	void SetVersion(const s_uint32_t aiVer);

	// 获取头部长度
	s_uint32_t GetHdrLen() const;

	void SetHdrLen(const s_uint32_t aiLen);

	s_uint8_t GetTos() const;

	void SetTos(const s_uint16_t aiTos);
	
	s_uint16_t GetTotalLen() const;

	void SetTotalLen(const s_uint16_t aiLen);

	s_uint16_t GetIdent() const;

	void SetIdent(const s_uint16_t aiId);

	s_uint16_t GetFragFlags() const;

	void SetFragFlags(const s_uint16_t aiFF);

	s_uint8_t GetTtl() const;

	void SetTtl(const s_uint16_t aiTtl);

	s_uint8_t GetProtocol() const;

	void SetProtocol(const s_uint16_t aiPro);

	s_uint16_t GetCheckSum() const;

	void SetCheckSum(const s_uint16_t aiSum);

	s_uint32_t TetSrcIp() const;

	void SetSrcIp(const s_uint32_t aiIp);

	s_uint32_t GetDstIp() const;

	void SetDstIp(const s_uint32_t aiIp);

protected:
	void* _pBuf; // 内容指向
};

class IcmpHdrHelper : public HdrHelper
{
public:
	IcmpHdrHelper(void* pBuf);

	s_uint8_t GetType() const;

	void SetType(const s_uint32_t aiType);

	s_uint8_t GetCode() const;

	void SetCode(const s_uint32_t aiCode);

	s_uint16_t GetCheckSum() const;

	void SetCheckSum(const s_uint16_t aiSum);

protected:
	void* _pBuf;
};

class IcmpEchoHelper : public IcmpHdrHelper
{
public:
	IcmpEchoHelper(void* buf);

	s_uint16_t GetIdent() const;

	void SetIdent(const s_uint16_t aiId);

	s_uint16_t GetSeq() const;

	void SetSeq(const s_uint16_t aiSeq);

	const char* GetOpt() const;

	void SetOpt(const char* pOpt, const s_uint32_t aiLen);
};

class IcmpTStampHelper : public IcmpHdrHelper
{
public:
	IcmpTStampHelper(void* pBuf);

	s_uint16_t GetIdent() const;

	void SetIdent(const s_uint16_t aiId);

	s_uint16_t GetSeq() const;

	void SetSeq(const s_uint16_t aiSeq);

	s_uint32_t GetReqTime() const;

	void SetReqTime(const s_uint32_t aiTime);

	s_uint32_t GetRecvTime() const;

	void SetRecvTime(const s_uint32_t aiTime);

	s_uint32_t GetTransTime() const;

	void SetTransTime(const s_uint32_t aiTime);
};

class IcmpMaskHelper : public IcmpHdrHelper
{
public:
	IcmpMaskHelper(void* pBuf);

	s_uint16_t GetIdent() const;

	void SetIdent(const s_uint16_t aiId);

	s_uint16_t GetSeq() const;

	void SetSeq(const s_uint16_t aiSeq);

	s_uint32_t GetMask() const;

	void SetMask(const s_uint32_t aiMask);
};

M_SOCKET_NAMESPACE_END
