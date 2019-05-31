#include "transfersvr/user_login_mgr.h"
#include "slience/base/logger.hpp"

void UserLoginMgr::Tick(const base::timestamp& now) {
	auto& time_index = _logininfo_container.get<LoginInfo::tag_tt>();
	for (auto iter = time_index.begin(); iter != time_index.end(); ++iter) {
		if (now.second() - iter->tt > 30) {
			LogInfo("del login userid:" 
				<< iter->userid
				<< " zone:"
				<< iter->zone
				<< " tt:"
				<< iter->tt
				<< " now:" 
				<< now.second()
			);
			time_index.erase(iter++);
		}
		else {
			iter++;
		}
	}
}

base::s_int32_t UserLoginMgr::GetLoginZone(base::s_uint64_t userid) {
	auto& id_index = _logininfo_container.get<LoginInfo::tag_userid>();
	auto iter = id_index.find(userid);
	if (iter == id_index.end()) {
		return 0;
	}
	return iter->zone;
}

base::s_int32_t UserLoginMgr::UpdateLoginInfo(base::s_uint64_t userid, base::s_int32_t zone, base::s_uint32_t tt) {
	base::s_int32_t old_zone = 0;
	auto& id_index = _logininfo_container.get<LoginInfo::tag_userid>();
	auto iter = id_index.find(userid);
	if (iter == id_index.end()) {
		LoginInfo info;
		info.userid = userid;
		info.tt = tt;
		info.zone = zone;
		_logininfo_container.insert(info);
	}
	else {
		old_zone = iter->zone;
		id_index.modify(iter, FuncModifyLogininfo(tt, zone));
	}
	LogInfo("update login userid:" << userid << " zone:" << zone << " tt:" << tt << " old_zone:" << old_zone);
	return old_zone;
}