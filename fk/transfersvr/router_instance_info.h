#pragma once

#include "slience/base/config.hpp"
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/ordered_index.hpp"

namespace bmi = boost::multi_index;

struct RouterInstInfo {
	base::s_int64_t fd;
	base::s_int32_t inst_id;
	bool online;

	struct tag_fd {};
	struct tag_inst_id{};
};

typedef bmi::multi_index_container<RouterInstInfo,
	bmi::indexed_by<
		bmi::ordered_non_unique<
			bmi::tag<RouterInstInfo::tag_fd>,
			bmi::member<RouterInstInfo, base::s_int64_t, &RouterInstInfo::fd>
		>,
		bmi::ordered_unique<
			bmi::tag<RouterInstInfo::tag_inst_id>,
			bmi::member<RouterInstInfo, base::s_int32_t, &RouterInstInfo::inst_id>
		>
	>
> RouterInstInfoContainer;


class FuncModifyRouterInstInfo {
public:
	FuncModifyRouterInstInfo(base::s_int64_t fd, bool online) {
		_fd = fd;
		_online = online;
	}

	void operator()(RouterInstInfo& info) {
		info.fd = _fd;
		info.online = _online;
	}

private:
	base::s_int64_t _fd;
	bool _online;
};