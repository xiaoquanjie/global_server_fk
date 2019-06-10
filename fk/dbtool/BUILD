cc_binary(
    name = 'dbtool',
    srcs = [
       'main.cpp',
	   'dbtool.pb.cc',
	   'mysql_executor.cpp',
    ],
    deps = [
		'//google:protobuf',
        '//commonlib:common',
		'//slience:slience',
		'//thirdparty/mariadb-connector-c-master:mysqlclient'
    ],
)
