#ifndef BASE_RANDOM_INCLUDE
#define BASE_RANDOM_INCLUDE

/**
伪随机数产生器头文件，采用线性同余算法
*/

#include "slience/base/config.hpp"
#include <ctime>

M_BASE_NAMESPACE_BEGIN

class random {
	typedef time_t SEED_TYPE;
public:

	random():m_is_rand_seed(true){}

	random(SEED_TYPE seed):m_seed(seed),m_is_rand_seed(false){}

	SEED_TYPE getSeed(){ return m_seed;}

	void setSeed(SEED_TYPE seed);

	/// 获得一个随机数，在0到1之间
	float rand();

	/// 获得一个随机数，在0, max之间，[0, max)
	int rand(int max); // [0, max)

	/// 获得一个随机数，在min, max之间，[min, max)
	int rand(int min, int max); // [min, max)

private:
	SEED_TYPE m_seed;
	bool      m_is_rand_seed;
};

void random::setSeed(SEED_TYPE seed) {
	m_seed = seed;
	m_is_rand_seed = false;
}

float random::rand() {
	if(m_is_rand_seed)
		m_seed = time(0);

	m_seed = (m_seed * 10807L) & 0x7fffffffL;
	float p = (float)m_seed / 0x7fffffffL;
	return p;
}

int random::rand(int max) {
	float p = rand();
	int r =(int)(max * p);
	if(r == max && max > 0)
		r = max - 1;

	return r;
}

int random::rand(int min, int max) {
	float p = rand();
	int r =(int)((max - min) * p + min);
	if(r == max && max > min)
		r = max - 1;
	if(r < min)
		r = min;

	return r;
}

M_BASE_NAMESPACE_END
#endif