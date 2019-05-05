#include "slience/socket/hdr_helper.hpp"
#include <assert.h>

M_SOCKET_NAMESPACE_BEGIN

s_uint16_t HdrHelper::CheckSum(const s_uint16_t* pAddr, const s_int32_t aiLen) {
	s_int32_t   liLeft = aiLen;
	s_uint32_t  liSum = 0;
	const s_uint16_t* pW = pAddr;
	s_uint16_t  liAnswer = 0;

	while (liLeft > 1)
	{
		liSum += *pW++;
		liLeft -= 2;
	}

	if (liLeft == 1)
	{
		*(s_uint8_t*)(&liAnswer) = *(s_uint8_t*)pW;
		liSum += liAnswer;
	}

	liSum = (liSum >> 16) + (liSum & 0xffff);
	liSum += liSum >> 16;
	liAnswer = (s_uint16_t)~liSum;
	return (liAnswer);
}

s_int32_t HdrHelper::GetIcmpTypeCnt() {
	return (sizeof(gIcmpTypeTab) / sizeof(icmp_type_t));
}

const icmp_type_t* HdrHelper::GetIcmpType(s_int32_t aiIdx) {
	if (aiIdx >= 0 && aiIdx < GetIcmpTypeCnt())
		return &gIcmpTypeTab[aiIdx];

	return 0;
}

///////////////////////////////////////////////////////////////////

IpHdrHelper::IpHdrHelper(void* pBuf) :_pBuf(pBuf) {
	assert(_pBuf);
}

// 获取版本号
s_uint32_t IpHdrHelper::GetVersion() const {
	const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
	s_uint8_t liVer = pHdr->HlenVer >> 4; // 取高4位
	return (liVer);
}

void IpHdrHelper::SetVersion(const s_uint32_t aiVer) {
	// 存到高4位
	ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
	s_uint8_t liVer = aiVer & 0xf;				// 截断
	liVer <<= 4;
	pHdr->HlenVer &= 0xf;
	pHdr->HlenVer |= liVer;
}

// 获取头部长度
s_uint32_t IpHdrHelper::GetHdrLen() const {
	const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
	s_uint8_t liLen = pHdr->HlenVer & 0xf; // 取低4位
	return (liLen << 2);
}

void IpHdrHelper::SetHdrLen(const s_uint32_t aiLen) {
	// 存到低4位
	ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
	s_uint32_t liLen = aiLen >> 2;
	s_uint8_t liLen2 = liLen & 0xf;	// 截断
	pHdr->HlenVer &= 0xf0;
	pHdr->HlenVer |= liLen2;
}

s_uint8_t IpHdrHelper::GetTos() const {
	const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
	return (pHdr->Tos);
}

void IpHdrHelper::SetTos(const s_uint16_t aiTos) {
	ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
	pHdr->Tos = (s_uint8_t)(aiTos);
}

s_uint16_t IpHdrHelper::GetTotalLen() const {
	const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
	return (g_ntohs(pHdr->TotalLen));
}

void IpHdrHelper::SetTotalLen(const s_uint16_t aiLen) {
	ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
	pHdr->TotalLen = g_htons(aiLen);
}

s_uint16_t IpHdrHelper::GetIdent() const {
	const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
	return (g_ntohs(pHdr->Ident));
}

void IpHdrHelper::SetIdent(const s_uint16_t aiId) {
	ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
	pHdr->Ident = g_htons(aiId);
}

s_uint16_t IpHdrHelper::GetFragFlags() const {
	const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
	return (g_ntohs(pHdr->FragFlags));
}

void IpHdrHelper::SetFragFlags(const s_uint16_t aiFF) {
	ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
	pHdr->FragFlags = g_htons(aiFF);
}

s_uint8_t IpHdrHelper::GetTtl() const {
	const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
	return (pHdr->Ttl);
}

void IpHdrHelper::SetTtl(const s_uint16_t aiTtl) {
	ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
	pHdr->Ttl = (s_uint8_t)(aiTtl);
}

s_uint8_t IpHdrHelper::GetProtocol() const {
	const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
	return (pHdr->Protocol);
}

void IpHdrHelper::SetProtocol(const s_uint16_t aiPro) {
	ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
	pHdr->Protocol = (s_uint8_t)(aiPro);
}

s_uint16_t IpHdrHelper::GetCheckSum() const {
	const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
	return (g_ntohs(pHdr->CheckSum));
}

void IpHdrHelper::SetCheckSum(const s_uint16_t aiSum) {
	ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
	pHdr->CheckSum = g_htons(aiSum);
}

s_uint32_t IpHdrHelper::TetSrcIp() const {
	const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
	return (g_ntohl(pHdr->SrcIp));
}

void IpHdrHelper::SetSrcIp(const s_uint32_t aiIp) {
	ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
	pHdr->SrcIp = g_htonl(aiIp);
}

s_uint32_t IpHdrHelper::GetDstIp() const {
	const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
	return (g_ntohl(pHdr->DstIp));
}

void IpHdrHelper::SetDstIp(const s_uint32_t aiIp) {
	ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
	pHdr->DstIp = g_htonl(aiIp);
}

///////////////////////////////////////////////////////////////////

IcmpHdrHelper::IcmpHdrHelper(void* pBuf) :_pBuf(pBuf) {
	assert(_pBuf);
}

s_uint8_t IcmpHdrHelper::GetType() const {
	const icmp_hdr_t* pHdr = (const icmp_hdr_t*)_pBuf;
	return (pHdr->Type);
}

void IcmpHdrHelper::SetType(const s_uint32_t aiType) {
	icmp_hdr_t* pHdr = (icmp_hdr_t*)_pBuf;
	pHdr->Type = (s_uint8_t)(aiType);
}

s_uint8_t IcmpHdrHelper::GetCode() const {
	const icmp_hdr_t* pHdr = (const icmp_hdr_t*)_pBuf;
	return (pHdr->Code);
}

void IcmpHdrHelper::SetCode(const s_uint32_t aiCode) {
	icmp_hdr_t* pHdr = (icmp_hdr_t*)_pBuf;
	pHdr->Code = (s_uint8_t)(aiCode);
}

s_uint16_t IcmpHdrHelper::GetCheckSum() const {
	const icmp_hdr_t* pHdr = (const icmp_hdr_t*)_pBuf;
	return (g_ntohs(pHdr->CheckSum));
}

void IcmpHdrHelper::SetCheckSum(const s_uint16_t aiSum) {
	icmp_hdr_t* pHdr = (icmp_hdr_t*)_pBuf;
	pHdr->CheckSum = g_htons(aiSum);
}

///////////////////////////////////////////////////////////////////

IcmpEchoHelper::IcmpEchoHelper(void* buf) :IcmpHdrHelper(buf) {}

s_uint16_t IcmpEchoHelper::GetIdent() const {
	const icmp_echo_hdr_t* pHdr = (const icmp_echo_hdr_t*)_pBuf;
	return (g_ntohs(pHdr->Ident));
}

void IcmpEchoHelper::SetIdent(const s_uint16_t aiId) {
	icmp_echo_hdr_t* pHdr = (icmp_echo_hdr_t*)_pBuf;
	pHdr->Ident = g_htons(aiId);
}

s_uint16_t IcmpEchoHelper::GetSeq() const {
	const icmp_echo_hdr_t* pHdr = (const icmp_echo_hdr_t*)_pBuf;
	return (g_ntohs(pHdr->Seq));
}

void IcmpEchoHelper::SetSeq(const s_uint16_t aiSeq) {
	icmp_echo_hdr_t* pHdr = (icmp_echo_hdr_t*)_pBuf;
	pHdr->Seq = g_htons(aiSeq);
}

const char* IcmpEchoHelper::GetOpt() const {
	const icmp_echo_hdr_t* pHdr = (const icmp_echo_hdr_t*)_pBuf;
	return (const char*)(pHdr + 1);
}

void IcmpEchoHelper::SetOpt(const char* pOpt, const s_uint32_t aiLen) {
	icmp_echo_hdr_t* pHdr = (icmp_echo_hdr_t*)_pBuf;
	char* p = (char*)(pHdr + 1);
	g_strncpy(p, pOpt, aiLen);
}

///////////////////////////////////////////////////////////////////

IcmpTStampHelper::IcmpTStampHelper(void* pBuf) :IcmpHdrHelper(pBuf) {}

s_uint16_t IcmpTStampHelper::GetIdent() const {
	const icmp_tstamp_hdr_t* pHdr = (const icmp_tstamp_hdr_t*)_pBuf;
	return (g_ntohs(pHdr->Ident));
}

void IcmpTStampHelper::SetIdent(const s_uint16_t aiId) {
	icmp_tstamp_hdr_t* pHdr = (icmp_tstamp_hdr_t*)_pBuf;
	pHdr->Ident = g_htons(aiId);
}

s_uint16_t IcmpTStampHelper::GetSeq() const {
	const icmp_tstamp_hdr_t* pHdr = (const icmp_tstamp_hdr_t*)_pBuf;
	return (g_ntohs(pHdr->Seq));
}

void IcmpTStampHelper::SetSeq(const s_uint16_t aiSeq) {
	icmp_tstamp_hdr_t* pHdr = (icmp_tstamp_hdr_t*)_pBuf;
	pHdr->Seq = g_htons(aiSeq);
}

s_uint32_t IcmpTStampHelper::GetReqTime() const {
	const icmp_tstamp_hdr_t* pHdr = (const icmp_tstamp_hdr_t*)_pBuf;
	return (g_ntohl(pHdr->ReqTime));
}

void IcmpTStampHelper::SetReqTime(const s_uint32_t aiTime) {
	icmp_tstamp_hdr_t* pHdr = (icmp_tstamp_hdr_t*)_pBuf;
	pHdr->ReqTime = g_htonl(aiTime);
}

s_uint32_t IcmpTStampHelper::GetRecvTime() const {
	const icmp_tstamp_hdr_t* pHdr = (const icmp_tstamp_hdr_t*)_pBuf;
	return (g_ntohl(pHdr->recv_time));
}

void IcmpTStampHelper::SetRecvTime(const s_uint32_t aiTime) {
	icmp_tstamp_hdr_t* pHdr = (icmp_tstamp_hdr_t*)_pBuf;
	pHdr->recv_time = g_htonl(aiTime);
}

s_uint32_t IcmpTStampHelper::GetTransTime() const {
	const icmp_tstamp_hdr_t* pHdr = (const icmp_tstamp_hdr_t*)_pBuf;
	return (g_ntohl(pHdr->TransTime));
}

void IcmpTStampHelper::SetTransTime(const s_uint32_t aiTime) {
	icmp_tstamp_hdr_t* pHdr = (icmp_tstamp_hdr_t*)_pBuf;
	pHdr->TransTime = g_htonl(aiTime);
}

///////////////////////////////////////////////////////////////////

IcmpMaskHelper::IcmpMaskHelper(void* pBuf) :IcmpHdrHelper(pBuf) {}

s_uint16_t IcmpMaskHelper::GetIdent() const {
	const icmp_mask_hdr_t* pHdr = (const icmp_mask_hdr_t*)_pBuf;
	return (g_ntohs(pHdr->Ident));
}

void IcmpMaskHelper::SetIdent(const s_uint16_t aiId) {
	icmp_mask_hdr_t* pHdr = (icmp_mask_hdr_t*)_pBuf;
	pHdr->Ident = g_htons(aiId);
}

s_uint16_t IcmpMaskHelper::GetSeq() const {
	const icmp_mask_hdr_t* pHdr = (const icmp_mask_hdr_t*)_pBuf;
	return (g_ntohs(pHdr->Seq));
}

void IcmpMaskHelper::SetSeq(const s_uint16_t aiSeq) {
	icmp_mask_hdr_t* pHdr = (icmp_mask_hdr_t*)_pBuf;
	pHdr->Seq = g_htons(aiSeq);
}

s_uint32_t IcmpMaskHelper::GetMask() const {
	const icmp_mask_hdr_t* pHdr = (const icmp_mask_hdr_t*)_pBuf;
	return (g_ntohl(pHdr->Mask));
}

void IcmpMaskHelper::SetMask(const s_uint32_t aiMask) {
	icmp_mask_hdr_t* pHdr = (icmp_mask_hdr_t*)_pBuf;
	pHdr->Mask = g_htonl(aiMask);
}

M_SOCKET_NAMESPACE_END