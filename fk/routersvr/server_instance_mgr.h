#ifndef M_SERVER_INSTANCE_MGR_INCLUDE
#define M_SERVER_INSTANCE_MGR_INCLUDE

#include "slience/base/singletion.hpp"
#include "protolib/src/routersvr_config.pb.h"
#include <set>
#include <unordered_map>

struct InstanceInfo {
	int svr_type;
	int policy;
	std::set<int> inst_vec;
};

struct SvrInfo {
	unsigned int svr_type;
	int svr_inst;

	struct HashOfKey {
	public:
		size_t operator()(const SvrInfo& info) const {
			size_t t = (info.svr_type << 10) + info.svr_inst;
			return t;
		}
	};

	struct EqualOfKey {
	public:
		bool operator() (const SvrInfo& rhs, const SvrInfo& lhs) const {
			return (rhs.svr_type == lhs.svr_type && rhs.svr_inst == lhs.svr_inst);
		}
	};
};


class SeverInstanceMgr {
public:
	typedef m_unorder_map_t<unsigned int, InstanceInfo> SvrType_InstInfo_Map;
	typedef m_unorder_map_t<base::s_int64_t, SvrInfo> Fd_SvrInfo_Map;
	typedef m_unorder_map_t<SvrInfo, base::s_int64_t,
		SvrInfo::HashOfKey, SvrInfo::EqualOfKey> SvrInfo_Fd_Map;

	SeverInstanceMgr();

	int Init(const config::RouterSvrConfig* conf);

	int AddInstance(unsigned int server_type, int instance_id, base::s_int64_t fd = 0);

	void DelInstance(unsigned int server_type, int instance_id);

	void DelInstance(base::s_int64_t fd);

	base::s_int64_t GetFdByTypeId(unsigned int server_type, int instance_id);

	SvrInfo* GetSvrInfoByFd(base::s_int64_t fd);

	void RouterPolicy(base::s_uint64_t uid, unsigned int server_type, bool is_broadcast, std::vector<int>& inst_vec);

protected:
	const config::Policy* GetPolicy(unsigned int server_type);

private:
	const config::RouterSvrConfig* _config;
	SvrType_InstInfo_Map _type_id_map;
	Fd_SvrInfo_Map _fd_svrinfo_map;
	SvrInfo_Fd_Map _svrinfo_fd_map;
};

#ifndef SeverInstanceMgrSgl
#define SeverInstanceMgrSgl base::singleton<SeverInstanceMgr>::mutable_instance()
#endif


#endif