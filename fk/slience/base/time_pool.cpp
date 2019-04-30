#include "slience/base/time_pool.h"
#include <map>
#include <list>
#include "slience/base/object_pool.h"

M_BASE_NAMESPACE_BEGIN

// 大桶的精度是1秒
static const int big_bucket_cnt = 24 * 3600;
// 不桶的精度是1/10秒
const int small_bucket_cnt = 10;

typedef std::map<base::s_int64_t, TimerPool::TimeNode*> TimeNodeMap;


TimerPool::TimerPool(int max_interval_day) {
	if (max_interval_day > 7 || max_interval_day < 0) {
		max_interval_day = 1;
	}
	_max_interval_day = max_interval_day;

	// 初始化大桶, 精度是1秒
	_bucket = (void**)malloc(sizeof(void*) * big_bucket_cnt * _max_interval_day);
	for (int idx = 0; idx < big_bucket_cnt; ++idx) {
		// 初始化小桶
		void** small_bucket_pointer = (void**)malloc(sizeof(void*) * small_bucket_cnt);
		memset(small_bucket_pointer, 0, sizeof(void*) * small_bucket_cnt);
		_bucket[idx] = (TimeNode*)small_bucket_pointer;
	}

	_beg_time = base::timestamp().millisecond();
	_big_bucket = 0;
	_small_bucket = 0;
	_cur_timer_id = 1;
}

TimerPool::~TimerPool() {
	for (int idx = 0; idx < big_bucket_cnt * _max_interval_day; ++idx) {
		for (int idx2 = 0; idx2 < small_bucket_cnt; ++idx2) {
			void** pp = (void**)_bucket[idx];
			TimeNodeMap* p = (TimeNodeMap*)pp[idx2];
			delete (p);
		}
		free(_bucket[idx]);
	}
	free(_bucket);
}

void TimerPool::Update(const base::timestamp& now) {
	int big_bucket = 0;
	int small_bucket = 0;
	_CalcBucket(now, 0, big_bucket, small_bucket);

	// 桶轮循
	std::list<TimeNode*> nodelist;
	int s_idx = _big_bucket * 10 + _small_bucket;
	int e_idx = big_bucket * 10 + small_bucket;
	for (; s_idx <= e_idx; ++s_idx) {
		int b = s_idx / 10;
		int s = s_idx % 10;
		void** pp = (void**)_bucket[b];
		TimeNodeMap* pmap = (TimeNodeMap*)(pp[s]);
		if (!pmap) {
			continue;
		}
		for (auto iter = pmap->begin(); iter != pmap->end();) {
			if (iter->first <= now.millisecond()) {
				nodelist.push_back(iter->second);
				pmap->erase(iter++);
			}
			else {
				++iter;
			}
		}
		for (auto iter = nodelist.begin(); iter != nodelist.end(); ++iter) {
			// callback
			(*iter)->cb();
			// recycle memory
			base::ObjectPool<TimeNode>::Dealloc(*iter);
		}
		nodelist.clear();
		if (pmap->empty()) {
			delete pmap;
			pp[s] = 0;
		}
	}
	_big_bucket = big_bucket;
	_small_bucket = small_bucket;
}

base::s_uint64_t TimerPool::AddTimer(int interval, m_function_t<void()> func) {
	if (interval <= 0) {
		return 0;
	}
	if (interval > big_bucket_cnt * _max_interval_day * 1000) {
		return 0;
	}

	base::timestamp now_time;
	int big_bucket = 0;
	int small_bucket = 0;
	_CalcBucket(now_time, interval, big_bucket, small_bucket);
	void** pp = (void**)_bucket[big_bucket];
	TimeNodeMap* pmap = (TimeNodeMap*)(pp[small_bucket]);
	if (!pmap) {
		pmap = new TimeNodeMap;
		pp[small_bucket] = pmap;
	}
	if (_cur_timer_id == 0xFFFFFFFE) {
		_cur_timer_id = 1;
	}

	base::s_uint64_t timer_id = (base::s_uint64_t)(big_bucket * 10 + small_bucket) << 32;
	timer_id += _cur_timer_id++;

	TimeNode* node = base::ObjectPool<TimeNode>::Alloc();
	node->expire = now_time.millisecond() + interval;
	node->cb = func;
	node->timer_id = timer_id;
	pmap->insert(std::make_pair(node->expire, node));

	return timer_id;
}

int TimerPool::CancelTimer(base::s_uint64_t id) {
	base::s_uint64_t flag = 0xFFFFFFFF;
	/*int low_32bit = (int)(id & flag);*/
	int high_32bit = (id >> 32) & flag;

	int big_bucket = high_32bit / 10;
	int small_bucket = high_32bit % 10;

	void** pp = (void**)_bucket[big_bucket];
	TimeNodeMap* pmap = (TimeNodeMap*)(pp[small_bucket]);
	if (!pmap) {
		return -1;
	}
	for (auto iter = pmap->begin(); iter != pmap->end();) {
		if (iter->second->timer_id == id) {
			base::ObjectPool<TimeNode>::Dealloc(iter->second);
			pmap->erase(iter);
			if (pmap->empty()) {
				delete pmap;
				pp[small_bucket] = 0;
			}
			return 0;
		}
		else {
			++iter;
		}
	}
	return -1;
}

bool TimerPool::_CalcBucket(const base::timestamp& now, int interval, int& big_bucket, int& small_bucket) {
	base::s_int64_t now_mil = now.millisecond();
	int inteval_mil = (int)(now_mil - _beg_time + interval);
	int interval_sec = inteval_mil / 1000;

	big_bucket = interval_sec % (big_bucket_cnt * _max_interval_day);
	small_bucket = ((inteval_mil % 1000) / 100) % 10;
	return true;
}

M_BASE_NAMESPACE_END