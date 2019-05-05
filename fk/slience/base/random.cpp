#include "slience/base/random.hpp"

M_BASE_NAMESPACE_BEGIN

void random::setSeed(SEED_TYPE seed) {
	m_seed = seed;
	m_is_rand_seed = false;
}

float random::rand() {
	if (m_is_rand_seed)
		m_seed = time(0);

	m_seed = (m_seed * 10807L) & 0x7fffffffL;
	float p = (float)m_seed / 0x7fffffffL;
	return p;
}

int random::rand(int max) {
	float p = rand();
	int r = (int)(max * p);
	if (r == max && max > 0)
		r = max - 1;

	return r;
}

int random::rand(int min, int max) {
	float p = rand();
	int r = (int)((max - min) * p + min);
	if (r == max && max > min)
		r = max - 1;
	if (r < min)
		r = min;

	return r;
}

M_BASE_NAMESPACE_END