需要添加宏：
WIN32
_WINDOWS
ENABLED_LOCAL_INFILE
HAVE_COMPRESS
LIBMARIADB
THREAD
MARIADB_SYSTEM_TYPE="Windows"
MARIADB_MACHINE_TYPE="AMD64"
HAVE_DLOPEN
_CRT_SECURE_NO_WARNINGS
HAVE_SCHANNEL
HAVE_TLS
SIZEOF_CHARP=4
CMAKE_INTDIR="Debug"

需要引用库：
ws2_32.lib
Shlwapi.lib

引用头文件：
..\mariadb-connector-c-master\zlib
..\mariadb-connector-c-master\win-iconv
..\mariadb-connector-c-master\libmariadb
..\mariadb-connector-c-master\plugins\auth
..\mariadb-connector-c-master\include
..\mariadb-connector-c-master\plugins\pvio