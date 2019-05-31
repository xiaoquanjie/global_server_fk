#pragma once

#include "slience/base/singletion.hpp"
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "slience/base/timer.hpp"

namespace bmi = boost::multi_index;

struct LoginInfo {
	base::s_uint64_t userid;
	base::s_uint32_t tt;
	base::s_int32_t zone;

	struct tag_userid{};
	struct tag_tt{};
};

typedef bmi::multi_index_container< LoginInfo,
	bmi::indexed_by<
		bmi::ordered_non_unique<
			bmi::tag<LoginInfo::tag_tt>,
			bmi::member<LoginInfo, base::s_uint32_t, &LoginInfo::tt>
		>,
		bmi::ordered_unique<
			bmi::tag<LoginInfo::tag_userid>,
			bmi::member<LoginInfo, base::s_uint64_t, &LoginInfo::userid>
		>
	>
> LoginInfoContainer;

class FuncModifyLogininfo {
public:
	FuncModifyLogininfo(base::s_uint32_t tt, base::s_int32_t zone) {
		_tt = tt;
		_zone = zone;
	}

	void operator()(LoginInfo& info) {
		info.tt = _tt;
		info.zone = _zone;
	}

private:
	base::s_uint32_t _tt;
	base::s_int32_t _zone;
};

class UserLoginMgr {
public:
	void Tick(const base::timestamp& now);

	base::s_int32_t GetLoginZone(base::s_uint64_t);

	base::s_int32_t UpdateLoginInfo(base::s_uint64_t userid, base::s_int32_t zone, base::s_uint32_t tt);

private:
	LoginInfoContainer _logininfo_container;
};

#ifndef UserLoginMgrSgl
#define UserLoginMgrSgl base::singleton<UserLoginMgr>::mutable_instance()
#endif