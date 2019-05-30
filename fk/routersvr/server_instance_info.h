#pragma once

#include "slience/base/singletion.hpp"
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include <set>

namespace bmi = boost::multi_index;

struct InstContainer {
	int svr_type;
	int policy;
	std::set<base::s_int32_t> inst_set;
};

struct SvrInstInfo {
	base::s_int32_t type_inst;
	base::s_int64_t fd;
	base::s_int32_t inst_id;
	bool online;
	shard_ptr_t<InstContainer> info;

	struct tag_fd{};
	struct tag_type_inst{};

	static base::s_int32_t CalcTypeId(base::s_int32_t type, base::s_int32_t id) {
		return (type << 10) + id;
	}
};

typedef bmi::multi_index_container<SvrInstInfo,
	bmi::indexed_by<
		bmi::ordered_unique<
				bmi::tag<SvrInstInfo::tag_type_inst>,
				bmi::member<SvrInstInfo, base::s_int32_t, &SvrInstInfo::type_inst>
		>,
		bmi::ordered_non_unique<
				bmi::tag<SvrInstInfo::tag_fd>,
				bmi::member<SvrInstInfo, base::s_int64_t, &SvrInstInfo::fd>
		>
	>
> SvrInstInfoContainer;
			

class FuncModifySvrInstInfo {
public:
	FuncModifySvrInstInfo(base::s_int64_t fd, bool online) {
		_fd = fd;
		_online = online;
	}

	void operator()(SvrInstInfo& info) {
		info.fd = _fd;
		info.online = _online;
	}

private:
	base::s_int64_t _fd;
	bool _online;
};



