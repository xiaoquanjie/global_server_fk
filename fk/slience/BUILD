
#proto_library(
#)

cc_library(
    name = 'slience',
    srcs = [
	   'base/buffer.cpp',
	   'base/condition.cpp',
	   'base/mutexlock.cpp',
	   'base/logger.cpp',
	   'base/string_util.cpp',
	   'base/timer.cpp',
	   'base/thread.cpp',
	   'base/object_pool.cpp',
	   'base/time_pool.cpp',
	   'base/util.cpp',
	   'base/random.cpp',
	   'socket/error.cpp',
	   'netio/sync_connector.cpp',
	   'netio/http_connector.cpp',
	   'netio/tcp_connector.cpp',
	   'netio/netio.cpp',
	   'netio/tcp_socket.cpp',
	   'netio/http_socket.cpp',
	   'coroutine/coroutine.cpp',
	   'coroutine/coroutine_linux.cpp',
	   'coroutine/coroutine_win.cpp',
	   'coroutine/courtouine_task.cpp'
    ],

    deps = [
		'#pthread',
    ],

    #link_all_symbols = True,
)


