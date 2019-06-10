cc_library(
	name = 'zookeeper',
	srcs = [
		'hashtable.c',
		'hashtable_itr.c',
		'recordio.c',
		'st_adaptor.c',
		'winport.c',
		'zk_hashtable.c',
		'zk_log.c',
		'zk_wrapper.cpp',
		'zookeeper.c',
		'zookeeper.jute.c',
	],
	
	deps = [
		'#pthread',
    ],

	defs = [
		#'THREADED',
		'USE_STATIC_LIB',
	],

	warning = False,
)
