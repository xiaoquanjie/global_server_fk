#pragma once
#include "dbtool/dbtool.pb.h"
#include "thirdparty/mysqlclient/ma_wrapper.h"

class MysqlExecutor {
public:
	int Execute(dbtool::MysqlSchemaConf& cfg);

protected:
	int CreateSchema(SqlConnectionPtr conn_ptr, const dbtool::MysqlSchema& schema);

	int CreateTable(SqlConnectionPtr conn_ptr, const std::string& schema_name, dbtool::MysqlTable& table);

	int GetTableFields(SqlConnectionPtr conn_ptr, const std::string& schema_name,
		const dbtool::MysqlTable& table, std::vector<std::string>& table_fields);

	int CreateNewTable(SqlConnectionPtr conn_ptr, const std::string& schema_name, const dbtool::MysqlTable& table);

	int CopyTable(SqlConnectionPtr conn_ptr, const std::string& schema_name, const std::string& newname, const std::string& oldname);

	int DropTable(SqlConnectionPtr conn_ptr, const std::string& schema_name, const std::string& name);

	int ChangeTable(SqlConnectionPtr conn_ptr, const std::string& schema_name, const dbtool::MysqlTable& table);

	std::string GetFieldTypeDesc(const dbtool::TableField& field);

	std::string GetFieldModifyTypeDesc(const dbtool::TableField& field);

	std::string GetFieldKeyDesc(const dbtool::TableKey& key);

	int RenameTableColumn(SqlConnectionPtr conn_ptr, const std::string& schema_name, const dbtool::MysqlTable& table);

	int ModifyTableColumn(SqlConnectionPtr conn_ptr, const std::string& schema_name, const dbtool::MysqlTable& table);

	int DeleteTableColumn(SqlConnectionPtr conn_ptr, const std::string& schema_name, const dbtool::MysqlTable& table);

	int AddTableColumn(SqlConnectionPtr conn_ptr, const std::string& schema_name, const dbtool::MysqlTable& table);

	int GetTableKeys(SqlConnectionPtr conn_ptr, const std::string& schema_name,
		const dbtool::MysqlTable& table, std::set<std::string>& table_keys);

	int ChangeKey(SqlConnectionPtr conn_ptr, const std::string& schema_name, const dbtool::MysqlTable& table);

protected:
	std::string _GetFieldTypeDesc(const dbtool::TableField& field, dbtool::MysqlFieldType type);
};
