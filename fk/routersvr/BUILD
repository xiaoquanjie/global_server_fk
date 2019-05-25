cc_binary(
    name = 'routersvr',
    srcs = [
       'main.cpp',
       'router_svr.cpp',
	   'server_instance_mgr.cpp',
	   'transfer_mgr.cpp',
	   'transaction/trans_application.cpp',
    ],
    deps = [
        '//commonlib:common',
		'//protolib:proto',
		'//hiredis_wrapper:hiredis_wrapper',
		'//slience:slience'
    ],
)
