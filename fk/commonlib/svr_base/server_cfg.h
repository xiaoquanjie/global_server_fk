#ifndef M_SVR_BASE_SERVER_CFG_INCLUDE
#define M_SVR_BASE_SERVER_CFG_INCLUDE

#include <fstream>
#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "slience/base/logger.hpp"

template<typename CfgType>
class ServerCfg {
public:
	int Parse(const char* file_path) {
		std::ifstream ifs(file_path, std::ios::in);
		if (!ifs) {
			LogError("open server cfg file failed, file_path=" << file_path);
			return -1;
		}
		google::protobuf::io::IstreamInputStream inputStream(&ifs);
		if (!google::protobuf::TextFormat::Parse(&inputStream, &_cfg)) {
			LogError("google::protobuf::TextFormat::Parse failed, file_path=" << file_path);
			ifs.close();
			return -2;
		}
		ifs.close();
		LogInfo("parse server cfg file success, file_path=" << file_path);
		return 0;
	}

	const CfgType& Data() const {
		return _cfg;
	}

	CfgType& Data() {
		return _cfg;
	}

private:
	CfgType _cfg;
};

#endif