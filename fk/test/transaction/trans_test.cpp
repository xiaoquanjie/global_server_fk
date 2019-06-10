#include "commonlib/transaction/base_transaction.h"
#include "commonlib/transaction/transaction_mgr.h"
#include "protolib/src/mytest.pb.h"
#include "thirdparty/hiredis_wrapper/wrapper/redis_wrapper.hpp"

class TransMysqlTestNotify
	: public BaseTransaction< TransMysqlTestNotify, mytest::MysqlTestNotify> {
public:
	TransMysqlTestNotify(unsigned int cmd) : BaseTransaction(cmd) {}

	int OnRequest(mytest::MysqlTestNotify& request) {
		LogInfo("begin notify.....");
		std::string url = "192.168.1.207:3306:user00:Iron@gbl:pld";
		std::string sql = "select * from account_info limit 10";
		int affected_rows = 0;
		int ret = MysqlQuery(userid(), url, sql, affected_rows, 4, [](int idx, char**row) {
			LogInfo(row[0] << ", " << row[1] << ", " << row[2] << ", " << row[3]);
		});
		LogInfo("ret:" << ret << " affected_rows:" << affected_rows);
		LogInfo("end notify.....");
		return 0;
	}
};

REGISTER__TEST_TRANSACTION(MYSQL_TEST_NOTIFY, TransMysqlTestNotify);

///////////////////////////////////////////////////////////////////////////////////

class TransRedisTestNotify
	: public BaseTransaction< TransRedisTestNotify, mytest::RedisTestNotify> {
public:
	TransRedisTestNotify(unsigned int cmd) : BaseTransaction(cmd) {}

	int OnRequest(mytest::RedisTestNotify& request) {
		LogInfo("begin notify.....");
		std::string url = "192.168.1.210:6379:::0";
		
		int ret = RedisExecute(userid(), url, GetRedisCmd("xiao"), [](RedisReplyParser& parser) {
			try {
				std::string value;
				parser.GetString(value);
				LogInfo("value:" << value);
			}
			catch (RedisException& e) {
				LogError(e.What());
			}
		});

		if (ret != 0) {
			LogError("failed to RedisExecute");
		}

		LogInfo("end notify.....");
		return 0;
	}
};

REGISTER__TEST_TRANSACTION(REDIS_TEST_NOTIFY, TransRedisTestNotify);