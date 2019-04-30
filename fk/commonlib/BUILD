
#proto_library(
#)

cc_library(
    name = 'common',
    srcs = [
	   'svr_base/ApplicationBase.cpp',
	   'svr_base/ApplicationFunc.cpp',
	   'svr_base/ApplicationBaseExt.cpp',
	   'svr_base/server_manager.cpp',
	   'svr_base/router_manager.cpp',
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
		'//mariadb-connector-c-master:mysqlclient',
		'//tinyxml:tinyxml',
		'//hiredis_wrapper:hiredis_wrapper',
    ],

    #link_all_symbols = True,
)


