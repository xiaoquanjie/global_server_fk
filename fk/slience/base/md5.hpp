#ifndef M_BASE_MD5_HPP__
#define M_BASE_MD5_HPP__

#include "slience/base/config.hpp"
#include <string>
#include <stdint.h> //  for data type

M_BASE_NAMESPACE_BEGIN

#include "md5_helper.ipp"

/* MD5 context. */ 
typedef struct _MD5_CTX{

	/* state (ABCD) */   
	/*四个32bits数，用于存放最终计算得到的消息摘要。当消息长度〉512bits时，也用于存放每个512bits的中间结果*/ 
	uint32 state[4];  

	/* number of bits, modulo 2^64 (lsb first) */    
	/*存储原始信息的bits数长度,不包括填充的bits，最长为 2^64 bits，因为2^64是一个64位数的最大值*/
	uint32 count[2];

	/* input buffer */ 
	/*存放输入的信息的缓冲区，512bits*/
	unsigned char buffer[64];

} MD5_CTX;

class MD5
{
public:
	MD5();

	MD5(const char* input);

	MD5(const MD5& md5);

	bool operator == (const MD5& md5);

	MD5& operator = (const MD5& md5);

	std::string toString();

	const unsigned char* get()const;

protected:

	void encrypt(const unsigned char* input);

	// 初始化
	void _init();

	void _update(const unsigned char* input, unsigned int inputLen);

	void _final();
private:

	MD5_CTX m_ctx;
	
	unsigned char m_digest[17];
};


MD5::MD5()
{
	R_memset((unsigned char*)m_digest,0,sizeof(m_digest));
	m_digest[16]='\0';
}

MD5::MD5(const char* input)
{
	encrypt((unsigned char*)input);
}

MD5::MD5(const MD5& md5)
{
	*this=md5;
}

bool MD5::operator==(const MD5& md5)
{
	return (R_memcmp(m_digest,md5.m_digest,sizeof(m_digest))==0);
}

MD5& MD5::operator=(const MD5& md5)
{
	R_memcpy(m_digest,md5.m_digest,sizeof(m_digest));
	return *this;
}

std::string MD5::toString()
{
	static const char hex_digits[] = "0123456789ABCDEF";
	char output[33];
	int index, j = 0;
	for (int i = 0; i < 16; i++) {
		index = (m_digest[i] & 0xF0) >> 4;
		output[j++] = hex_digits[index];
		index = m_digest[i] & 0x0F;
		output[j++] = hex_digits[index];
	}
	output[j] = 0;
	return std::string(output);
}

const unsigned char* MD5::get()const
{
	return m_digest;
}

void MD5::encrypt(const unsigned char* input)
{
	R_memset((unsigned char*)m_digest,0,sizeof(m_digest));
	m_digest[16]='\0';

	_init();

	_update(input,R_strlen((char*)input));

	_final();
}

void MD5::_init()
{
	/*将当前的有效信息的长度设成0,这个很简单,还没有有效信息,长度当然是0了*/
	m_ctx.count[0] = m_ctx.count[1] = 0;

	/* Load magic initialization constants.*/
	/*初始化链接变量，算法要求这样，这个没法解释了*/
	m_ctx.state[0] = 0x67452301;
	m_ctx.state[1] = 0xefcdab89;
	m_ctx.state[2] = 0x98badcfe;
	m_ctx.state[3] = 0x10325476;
}

void MD5::_update(const unsigned char* input, unsigned int inputLen)
{
	unsigned int i, index, partLen;

	/* Compute number of bytes mod 64 */
	/*计算已有信息的bits长度的字节数的模64, 64bytes=512bits。
	用于判断已有信息加上当前传过来的信息的总长度能不能达到512bits，
	如果能够达到则对凑够的512bits进行一次处理*/
	index = (unsigned int)((m_ctx.count[0] >> 3) & 0x3F);

	/* Update number of bits *//*更新已有信息的bits长度*/
	if((m_ctx.count[0] += ((uint32)inputLen << 3)) < ((uint32)inputLen << 3))
		m_ctx.count[1]++;
	m_ctx.count[1] += ((uint32)inputLen >> 29);

	/*计算已有的字节数长度还差多少字节可以 凑成64的整倍数*/
	partLen = 64 - index;

	/* Transform as many times as possible.
	*/
	/*如果当前输入的字节数 大于 已有字节数长度补足64字节整倍数所差的字节数*/
	if(inputLen >= partLen) 
	{
		/*用当前输入的内容把context->buffer的内容补足512bits*/
		R_memcpy((POINTER)&m_ctx.buffer[index], (POINTER)input, partLen);
		/*用基本函数对填充满的512bits（已经保存到context->buffer中） 做一次转换，转换结果保存到context->state中*/
		MD5Helper::MD5Transform(m_ctx.state,m_ctx.buffer);
		/*
		对当前输入的剩余字节做转换（如果剩余的字节<在输入的input缓冲区中>大于512bits的话 ），
		转换结果保存到context->state中
		*/
		for(i = partLen; i + 63 < inputLen; i += 64 )/*把i+63<inputlen改为i+64<=inputlen更容易理解*/
			MD5Helper::MD5Transform(m_ctx.state, &input[i]);

		index = 0;
	}
	else
		i = 0;

	/* Buffer remaining input */
	/*将输入缓冲区中的不足填充满512bits的剩余内容填充到context->buffer中，留待以后再作处理*/
	R_memcpy((POINTER)&m_ctx.buffer[index], (POINTER)&input[i], inputLen-i);
}

void MD5::_final()
{
	unsigned char bits[8];
	uint32 index, padLen;

	/* Save number of bits */
	/*将要被转换的信息(所有的)的bits长度拷贝到bits中*/
	MD5Helper::Encode(bits,m_ctx.count,8);

	/* Pad out to 56 mod 64. */
	/* 计算所有的bits长度的字节数的模64, 64bytes=512bits*/
	index = (uint32)((m_ctx.count[0] >> 3) & 0x3f);
	
	/*计算需要填充的字节数，padLen的取值范围在1-64之间*/
	padLen = (index < 56) ? (56 - index) : (120 - index);
	
	/*这一次函数调用绝对不会再导致MD5Transform的被调用，因为这一次不会填满512bits*/
	_update(MD5Helper::PADDING,padLen);
	
	/* Append length (before padding) */
	/*补上原始信息的bits长度（bits长度固定的用64bits表示），这一次能够恰巧凑够512bits，不会多也不会少*/
	_update(bits, 8);
	
	/* Store state in digest */
	/*将最终的结果保存到digest中。ok，终于大功告成了*/
	MD5Helper::Encode(m_digest, m_ctx.state,16);

	/* Zeroize sensitive information. */
	R_memset((void*)(&m_ctx), 0, sizeof(MD5_CTX));
}

M_BASE_NAMESPACE_END
#endif