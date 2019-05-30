cc_binary(
    name = 'transfersvr',
    srcs = [
       'main.cpp',
       'transfersvr.cpp',
	   'transaction/trans_application.cpp',
	   'router_mgr.cpp',
	   'transfer_mgr.pp',
    ],
    deps = [
        '//commonlib:common',
		'//protolib:proto',
		'//hiredis_wrapper:hiredis_wrapper',
		'//slience:slience'
    ],
)
