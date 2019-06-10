
#proto_library(
#)

cc_library(
    name = 'common',
    srcs = [
	   'svr_base/ApplicationBase.cpp',
	   'svr_base/ApplicationFunc.cpp',
	   'svr_base/ApplicationBaseExt.cpp',
	   'net_handler/base_net_handler.cpp',
	   'net_handler/net_handler.cpp',
	   'net_handler/router_mgr.cpp',
	   'transaction/transaction_mgr.cpp',
	   'transaction/base_transaction.cpp',
	   'async/async_mysql_mgr.cpp',
	   'async/async_redis_mgr.cpp',
    ],

    deps = [
		'//slience:slience',
		'#pthread',
		'//thirdparty/mariadb-connector-c-master:mysqlclient',
		'//tinyxml:tinyxml',
		'//thirdparty/hiredis_wrapper:hiredis_wrapper',
		'//thirdparty/zookeeper:zookeeper'
    ],

    #link_all_symbols = True,
)


