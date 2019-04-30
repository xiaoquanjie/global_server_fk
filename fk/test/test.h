#pragma once

#include "slience/base/singletion.hpp"
#include "commonlib/svr_base/ApplicationBase.h"


class TestApplication : public ApplicationBase {
protected:
	int ServerType() override;

	int InstanceId() override;

	int OnInitNetWork() override;

	void OnStopNetWork() override;

	int UpdateNetWork() override;

	bool UseAsyncMysql() override;

	bool UseAsyncRedis() override;

	int OnTick(const base::timestamp& now);
};

typedef base::singleton<TestApplication> TestAppSgl;