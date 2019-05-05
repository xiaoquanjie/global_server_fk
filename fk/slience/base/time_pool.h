/*
* 时间定时器
* 
*
*
*/
#pragma once

#include "slience/base/compatibility.hpp"
#include "slience/base/timer.hpp"

M_BASE_NAMESPACE_BEGIN


class TimerPool {
public:
	// 时间节点
	struct TimeNode {
		base::s_uint64_t timer_id;
		base::s_int64_t expire;	// 超时时间(毫秒)
		m_function_t<void()> cb;	// 回调函数
	};

	// @max_interval_day 最大不能超过7天，默认是1天（24小时)
	TimerPool(int max_interval_day = 1);

	~TimerPool();

	// 调用此函数，超时的节点会被调用回调
	void Update(const base::timestamp& now);

	// @interval是毫秒, 返回值是timer id, 值大于0
	base::s_uint64_t AddTimer(int interval, m_function_t<void()> func);

	int CancelTimer(base::s_uint64_t id);

protected:
	bool _CalcBucket(const base::timestamp& now, int interval, int& big_bucket, int& small_bucket);

private:
	int _max_interval_day;	// 最大的时间间隔（天数)
	void** _bucket;			// 桶，精度是毫秒
	base::s_int64_t _beg_time;
	int _big_bucket;
	int _small_bucket;
	base::s_uint32_t _cur_timer_id;
};


M_BASE_NAMESPACE_END
