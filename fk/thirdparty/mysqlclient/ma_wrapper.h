#ifndef _MA_WRAPPER_H
#define _MA_WRAPPER_H

#include <string>
#include <string.h>
#ifdef WIN32
#include <memory>
#include <functional>
#include <Windows.h>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "Shlwapi.lib")
#else
#include <tr1/memory>
#include <tr1/functional>
#include <pthread.h>
#endif
#include "thirdparty/mariadb-connector-c-master/include/mysql.h"
#include "thirdparty/mariadb-connector-c-master/include/errmsg.h"
#include "thirdparty/mariadb-connector-c-master/include/mysqld_error.h"
#include <unordered_map>
#include <assert.h>
#include <sstream>
#include "google/protobuf/repeated_field.h"
#include "google/protobuf/descriptor.h"

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
				pthread_key_create(&_tkey, destructor);
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
		static void destructor(void* v) {
			T* pv = (T*)v;
			delete pv;
		}
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

	int Execute(const char* sql, unsigned int sql_len) {
		int affected_rows = 0;
		return Execute(sql, sql_len, affected_rows);
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
				break;;
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

			affected_rows = (int)mysql_num_rows(res);
			if (affected_rows < 0) {
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
	
	template<typename T>
	int QueryToProtobuf(const char* sql, unsigned int sql_len, T& value_list);

	template<typename T>
	int QueryToRepeated(const char* sql, unsigned int sql_len, ::google::protobuf::RepeatedPtrField<T>& value_list);

	MYSQL_RES* StoreResult() {
		MYSQL_RES* res = mysql_store_result(&_st_mysql);
		return res;
	}

	int Autocommit(bool open_or_close) {
		int ret = mysql_autocommit(&_st_mysql, open_or_close);
		if (ret != 0) {
			unsigned int code = mysql_errno(&_st_mysql);
			if (CR_SERVER_GONE_ERROR == code
				|| CR_SERVER_LOST == code) {
				if (mariadb_reconnect(&_st_mysql)) {
					ret = mysql_autocommit(&_st_mysql, open_or_close);
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
		return ret;
	}

	int StartTransaction() {
		std::string sql = "START TRANSACTION;";
		return Execute(sql.c_str(), sql.length());
	}

	int Rollback() {
		return mysql_rollback(&_st_mysql);
	}

	int Commit() {
		return mysql_commit(&_st_mysql);
	}
protected:
	bool _UseDb(const char* new_db) {
		int ret = mysql_select_db(&_st_mysql, new_db);
		if (ret != 0) {
			unsigned int code = mysql_errno(&_st_mysql);
			if (CR_SERVER_GONE_ERROR == code
				|| CR_SERVER_LOST == code) {
				if (mariadb_reconnect(&_st_mysql)) {
					ret = mysql_select_db(&_st_mysql, new_db);
				}
				else {
					_set_errno(CR_SERVER_LOST);
					_set_err_msg("reconnect fail");
					return false;
				}
			}
		}
		return (ret == 0);
	}

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

	void _set_unistr(const std::string& str) {
		_unistr = str;
	}

	std::string _get_unistr() const {
		return _unistr;
	}

private:
	MYSQL _st_mysql;
	char _err_msg[256];
	unsigned int _errno;
	std::string _unistr;
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
		std::string uni_str = _CalcUniStr(host, user, passwd, db, port);
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
				ptr->_set_unistr(uni_str);
			} while (false);

			if (!ptr) {
				snprintf(info.error_msg, sizeof(info.error_msg), "%s", mysql_error(&conn->_st_mysql));
				delete conn;
			}
			return ptr;
		}
	}

	static bool UseDb(SqlConnectionPtr ptr, const char* new_db) {
		if (!ptr) {
			return false;
		}
		std::string old_unistr = ptr->_get_unistr();
		std::string host, user, passwd, db, port;
		if (!_AnalyseUniStr(old_unistr, host, user, passwd, db, port)) {
			return false;
		}

		if (!ptr->_UseDb(new_db)) {
			return false;
		}

		std::string new_unistr = _CalcUniStr(host.c_str(), user.c_str(), 
			passwd.c_str(), new_db, atoi(port.c_str()));
		ptr->_set_unistr(new_unistr);
		_mysqlinfo_& info = _mysql_detail::ThreadLocalData<_mysqlinfo_>::data();
		info.info[new_unistr] = ptr;
		info.info.erase(old_unistr);
		return true;
	}

	static const char* GetErrorMsg() {
		_mysqlinfo_& info = _mysql_detail::ThreadLocalData<_mysqlinfo_>::data();
		return info.error_msg;
	}

protected:
	static std::string _CalcUniStr(const char* host, const char* user, const char* passwd,
		const char* db, unsigned short port) {
		std::string uni_str = host;
		uni_str += ":";
		uni_str += user;
		uni_str += ":";
		uni_str += passwd;
		uni_str += ":";
		uni_str += db;
		uni_str += ":";
		uni_str += std::to_string(port);
		return uni_str;
	}

	static bool _AnalyseUniStr(const std::string& str, std::string& host, std::string& user, std::string& passwd,
		std::string& db, std::string& port) {
		std::string separator = ":";
		std::vector<std::string> array;
		std::string::size_type start = 0;
		while (true) {
			std::string::size_type pos = str.find_first_of(separator, start);
			if (pos == std::string::npos) {
				std::string sub = str.substr(start, str.size());
				if (!sub.empty())
					array.push_back(sub.c_str());
				else
					array.push_back("");
				break;
			}

			std::string sub = str.substr(start, pos - start);
			start = pos + separator.size();
			if (!sub.empty())
				array.push_back(sub.c_str());
			else
				array.push_back("");
		}

		if (array.size() != 5) {
			return false;
		}
		else {
			host = array[0];
			user = array[1];
			passwd = array[2];
			db = array[3];
			port = array[4];
			return true;
		}
	}
};

//////////////////////////////////////////////////////////////////

template<typename T>
int SqlConnection::QueryToProtobuf(const char* sql, unsigned int sql_len, T& value_list) {
	auto repeated_items = value_list.mutable_items();
	return QueryToRepeated(sql, sql_len, *repeated_items);
}

template<typename T>
int SqlConnection::QueryToRepeated(const char* sql, unsigned int sql_len, ::google::protobuf::RepeatedPtrField<T>& value_list) {
	auto desc = T::descriptor();
	int ret = Query(sql, sql_len, desc->field_count(),
		[&value_list, desc](int Index, MYSQL_ROW row) {
		auto items = value_list.Add();
		auto reflection = items->GetReflection();
		for (int idx = 0; idx < desc->field_count(); ++idx) {
			auto field = desc->field(idx);
			switch (field->type()) {
			case google::protobuf::FieldDescriptor::TYPE_INT32: {
				std::istringstream iss(row[idx]);
				google::protobuf::int32 v;
				iss >> v;
				reflection->SetInt32(items, field, v);
			}
				break;
			case google::protobuf::FieldDescriptor::TYPE_UINT32: {
				std::istringstream iss(row[idx]);
				google::protobuf::uint32 v;
				iss >> v;
				reflection->SetUInt32(items, field, v);
			}
				break;
			case google::protobuf::FieldDescriptor::TYPE_INT64: {
				std::istringstream iss(row[idx]);
				google::protobuf::int64 v;
				iss >> v;
				reflection->SetInt64(items, field, v);
			}
				break;
			case google::protobuf::FieldDescriptor::TYPE_UINT64: {
				std::istringstream iss(row[idx]);
				google::protobuf::uint64 v;
				iss >> v;
				reflection->SetUInt64(items, field, v);
			}
				break;
			case google::protobuf::FieldDescriptor::TYPE_DOUBLE: {
				std::istringstream iss(row[idx]);
				double v;
				iss >> v;
				reflection->SetDouble(items, field, v);
			}
				break;
			case google::protobuf::FieldDescriptor::TYPE_FLOAT: {
				std::istringstream iss(row[idx]);
				float v;
				iss >> v;
				reflection->SetFloat(items, field, v);
			}
				break;
			case google::protobuf::FieldDescriptor::TYPE_BOOL: {
				std::istringstream iss(row[idx]);
				bool v;
				iss >> v;
				reflection->SetBool(items, field, v);
			}
				break;
			case google::protobuf::FieldDescriptor::TYPE_STRING: {
				reflection->SetString(items, field, row[idx]);
			}
				break;
			case google::protobuf::FieldDescriptor::TYPE_BYTES: {
				reflection->SetString(items, field, row[idx]);
			}
				break;
			default:
				break;
			}
		}
	});
	return ret;
}

#endif