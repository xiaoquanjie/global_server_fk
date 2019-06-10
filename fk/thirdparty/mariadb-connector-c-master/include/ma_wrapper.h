#ifndef _MA_WRAPPER_H
#define _MA_WRAPPER_H

#include <string>
#include <string.h>
#ifdef WIN32
#include <memory>
#include <functional>
#include <Windows.h>
#else
#include <tr1/memory>
#include <tr1/functional>
#include <pthread.h>
#endif
#include "mysql.h"
#include "errmsg.h"
#include "mysqld_error.h"
#include <unordered_map>
#include <assert.h>

// 线程局部对象
namespace _mysql_detail {
#ifdef WIN32
	template<typename T>
	class ThreadLocalData
	{
	public:
		struct _init_ {
			DWORD _tkey;
			_init_() {
				// 创建
				_tkey = TlsAlloc();
				assert(_tkey != TLS_OUT_OF_INDEXES);
			}
			~_init_() {
				TlsFree(_tkey);
			}
		};

		inline static T& data() {
			T* pv = 0;
			if (0 == (pv = (T*)TlsGetValue(_data._tkey))) {
				pv = new T;
				TlsSetValue(_data._tkey, (void*)pv);
			}
			return *pv;
		}

	protected:
		ThreadLocalData(const ThreadLocalData&);
		ThreadLocalData& operator=(const ThreadLocalData&);

	private:
		static _init_ _data;
	};

#else
	template<typename T>
	class ThreadLocalData
	{
	public:
		struct _init_ {
			pthread_key_t _tkey;
			_init_() {
				// 创建
				pthread_key_create(&_tkey, 0);
			}
			~_init_() {
				T* pv = (T*)pthread_getspecific(_tkey);
				pthread_key_delete(_tkey);
				delete pv;
			}
		};

		inline static T& data() {
			T* pv = 0;
			if (0 == (pv = (T*)pthread_getspecific(_data._tkey))) {
				pv = new T;
				pthread_setspecific(_data._tkey, (void*)pv);
			}
			return *pv;
		}

	protected:
		ThreadLocalData(const ThreadLocalData&);
		ThreadLocalData& operator=(const ThreadLocalData&);

	private:
		static _init_ _data;
	};
#endif
	template<typename T>
	typename ThreadLocalData<T>::_init_ ThreadLocalData<T>::_data;
}

///////////////////////////////////////////////////////////////////

class MysqlPool;
class SqlConnection;

#ifdef WIN32
typedef std::shared_ptr<SqlConnection> SqlConnectionPtr;
typedef std::function<void(int, MYSQL_ROW)> RowCallBack;
#else
typedef std::tr1::shared_ptr<SqlConnection> SqlConnectionPtr;
typedef std::tr1::function<void(int, MYSQL_ROW)> RowCallBack;
#endif

class SqlConnection {
	friend class MysqlPool;
public:
	~SqlConnection() {
		mysql_close(&_st_mysql);
	}

	const char* GetErrorMsg() {
		return _err_msg;
	}

	unsigned int GetErrorNo() {
		return _errno;
	}

	// 是否是键值重复
	bool IsDupEntry() {
		return (_errno == ER_DUP_ENTRY);
	}

	int Execute(const std::string& sql, int& affected_rows) {
		return Execute(sql.c_str(), sql.length(), affected_rows);
	}

	int Execute(const char* sql, unsigned int sql_len, int& affected_rows) {
		affected_rows = 0;
		int ret = mysql_real_query(&_st_mysql, sql, sql_len);
		if (0 != ret) {
			unsigned int code = mysql_errno(&_st_mysql);
			if (CR_SERVER_GONE_ERROR == code
				|| CR_SERVER_LOST == code) {
				if (mariadb_reconnect(&_st_mysql)) {
					ret = mysql_real_query(&_st_mysql, sql, sql_len);
				}
				else {
					_set_errno(CR_SERVER_LOST);
					_set_err_msg("reconnect fail");
					return -1;
				}
			}
		}

		if (0 != ret) {
			_set_err_msg();
			_set_errno();
		}
		else {
			affected_rows = (int)mysql_affected_rows(&_st_mysql);
		}
		return ret;
	}

	int Query(const std::string& sql, int expected_fields, RowCallBack callback) {
		int row_cnt = 0;
		return Query(sql.c_str(), sql.length(), expected_fields, row_cnt, callback);
	}

	int Query(const std::string& sql, int expected_fields, int& row_cnt, RowCallBack callback) {
		return Query(sql.c_str(), sql.length(), expected_fields, row_cnt, callback);
	}

	int Query(const char* sql, unsigned int sql_len, int expected_fields, RowCallBack callback) {
		int row_cnt = 0;
		return Query(sql, sql_len, expected_fields, row_cnt, callback);
	}

	int Query(const char* sql, unsigned int sql_len, int expected_fields, int& row_cnt, RowCallBack callback) {
		int affected_rows = 0;
		int ret = Execute(sql, sql_len, affected_rows);
		if (0 != ret) {
			return ret;
		}

		MYSQL_RES* res = 0; 
		do {
			res = mysql_store_result(&_st_mysql);
			if (!res) {
				ret = -1;
				break;
			}

			int fields_cnt = mysql_num_fields(res);
			if (fields_cnt < 0 || fields_cnt != expected_fields) {
				std::string tmp = "expected_fields count not match {expected: ";
				tmp += std::to_string(expected_fields);
				tmp += " real:" + std::to_string(fields_cnt);
				tmp += "}";
				_set_err_msg(tmp.c_str());
				ret = -2;
				break;
			}

			row_cnt = (int)mysql_num_rows(res);
			if (row_cnt < 0) {
				ret = -3;
				break;
			}

			int idx = 0;
			MYSQL_ROW row = 0;
			while (0 != (row = mysql_fetch_row(res))) {
				callback(idx, row);
				++idx;
			}

		} while (false);

		if (res) {
			mysql_free_result(res);
		}
		if (0 != ret && -2 != ret) {
			_set_err_msg();
			_set_errno();
		}
		return ret;
	}

protected:
	SqlConnection() {
		_errno = 0;
		memset(_err_msg, 0, sizeof(_err_msg));
	}

	void _set_err_msg(const char* msg) {
		snprintf(_err_msg, sizeof(_err_msg), "%s", msg);
	}

	void _set_err_msg() {
		snprintf(_err_msg, sizeof(_err_msg), "%s", mysql_error(&_st_mysql));
	}

	void _set_errno(unsigned int no) {
		_errno = no;
	}

	void _set_errno() {
		_errno = mysql_errno(&_st_mysql);
	}

private:
	MYSQL _st_mysql;
	char _err_msg[256];
	unsigned int _errno;
};


///////////////////////////////////////////////////////////////////

struct _mysqlinfo_ {
	_mysqlinfo_() {
		memset(error_msg, 0, sizeof(error_msg));
	}
	char error_msg[256];
	std::unordered_map<std::string, SqlConnectionPtr> info;
};

///////////////////////////////////////////////////////////////////

class MysqlPool {
public:
	static SqlConnectionPtr GetConnection(const char* host, const char* user, const char* passwd,
		const char* db, unsigned short port = 3306) {
		_mysqlinfo_& info = _mysql_detail::ThreadLocalData<_mysqlinfo_>::data();
		if (!host || !user || !passwd || !db) {
			snprintf(info.error_msg, sizeof(info.error_msg), "%s", "some param is empty");
			return SqlConnectionPtr();
		}
		std::string uni_str = host;
		uni_str += user;
		uni_str += passwd;
		uni_str += db;
		uni_str += std::to_string(3306);

		auto iter = info.info.find(uni_str);
		if (iter != info.info.end()) {
			return iter->second;
		}
		else {
			SqlConnectionPtr ptr;
			SqlConnection* conn = new SqlConnection;
			do {
				if (!mysql_init(&conn->_st_mysql)) {
					break;
				}

				char reconn_flag = 1;
				if (0 != mysql_options(&conn->_st_mysql, MYSQL_OPT_RECONNECT, &reconn_flag)) {
					break;
				}
				if (0 != mysql_options(&conn->_st_mysql, MYSQL_SET_CHARSET_NAME, "utf8")) {
					break;
				}

				int conn_flag = CLIENT_FOUND_ROWS;
				if (!mysql_real_connect(&conn->_st_mysql, host, user, passwd, db, port, NULL,
					conn_flag)) {
					break;
				}
				ptr.reset(conn);
				info.info[uni_str] = ptr;
			} while (false);

			if (!ptr) {
				snprintf(info.error_msg, sizeof(info.error_msg), "%s", mysql_error(&conn->_st_mysql));
				delete conn;
			}
			return ptr;
		}
	}

	static const char* GetErrorMsg() {
		_mysqlinfo_& info = _mysql_detail::ThreadLocalData<_mysqlinfo_>::data();
		return info.error_msg;
	}
};

#endif