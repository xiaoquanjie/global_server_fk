#pragma once

#include "slience/base/config.hpp"
#include <string>

M_BASE_NAMESPACE_BEGIN

time_t HmsToSecond(const std::string& hms);

// 一天的固定时间
time_t CertainTimesOfDay(time_t now, const std::string& hms);

// 最接近的时间段
time_t ClosestTimeOfDay(time_t now, time_t peroid);

base::s_uint64_t ntohll(base::s_uint64_t val);

base::s_uint64_t htonll(base::s_uint64_t val);

M_BASE_NAMESPACE_END