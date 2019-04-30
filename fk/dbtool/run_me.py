#! /usr/bin/env python
# coding=utf-8
import os
import sys
import platform
import argparse
import google.protobuf


def gen_pb():
    proto_files = ['dbtool.proto']
    for f in proto_files:
        cpp_cmd = None
        python_cmd = None
        if platform.system() == 'Windows':
            cpp_cmd = '..\\tool\\protoc.exe  --proto_path=../dbtool/  --cpp_out=../dbtool/ '
            cpp_cmd += f
            python_cmd = '..\\tool\\protoc.exe  --proto_path=../dbtool/  --python_out=../dbtool/ '
            python_cmd += f
        else:
            cpp_cmd = "protoc --proto_path=./ --cpp_out=./ " + f
            python_cmd = "protoc --proto_path=./ --python_out=./ " + f

        os.system(cpp_cmd)
        os.system(python_cmd)


class Table:
    def __init__(self):
        self.schema = None
        self.name = None
        self.column_str = []

    def to_string(self):
        col_str = 'message ' + self.name + ' {\n'
        for col in self.column_str:
            col_str += '    ' + col + ';\n'
        col_str += '}\n'
        return col_str


def gen_proto(args):
    import dbtool_pb2
    table_map = {}
    for conf in args.conf:
        schema_conf = dbtool_pb2.MysqlSchemaConf()
        google.protobuf.text_format.Parse(open(conf).read(), schema_conf)
        print('---------------------%s----------------' % conf)
        for schema in schema_conf.mysql_schemas:
            print('~~~~~~~~~~~~~~~~~~%s~~~~~~~~~~~~~~~~~~~' % schema.schema_name)
            if schema.schema_name not in table_map:
                table_map[schema.schema_name] = []
            for t in schema.tables:
                table = Table()
                table.schema = schema.schema_name
                table.name = t.table_name
                idx = 1;
                for field in t.fields:
                    if field.is_delete:
                        continue
                    field_type = None
                    if field.type == dbtool_pb2.E_FieldType_Int:
                        field_type = 'int32'
                    elif field.type == dbtool_pb2.E_FieldType_TinyInt:
                        field_type = 'int32'
                    elif field.type == dbtool_pb2.E_FieldType_UInt:
                        field_type = 'uint32'
                    elif field.type == dbtool_pb2.E_FieldType_BigInt:
                        field_type = 'int64'
                    elif field.type == dbtool_pb2.E_FieldType_Double:
                        field_type = 'double'
                    elif field.type == dbtool_pb2.E_FieldType_Varchar:
                        field_type = 'string'
                    elif field.type == dbtool_pb2.E_FieldType_Blob:
                        field_type = 'string'
                    elif field.type == dbtool_pb2.E_FieldType_MediumBlob:
                        field_type = 'string'
                    elif field.type == dbtool_pb2.E_FieldType_Text:
                        field_type = 'string'
                    elif field.type == dbtool_pb2.E_FieldType_Date:
                        field_type = 'string'
                    elif field.type == dbtool_pb2.E_FieldType_Time:
                        field_type = 'string'
                    elif field.type == dbtool_pb2.E_FieldType_TimeStamp:
                        field_type = 'string'
                    else:
                        print(field.type)
                    col = 'optional ' + field_type + ' ' + field.name + ' = ' + str(idx)
                    idx += 1
                    table.column_str.append(col)

                print(table.to_string())
                table_map[schema.schema_name].append(table)
                table2 = Table()
                table2.schema = schema.schema_name
                table2.name = table.name + '_list'
                table2.column_str.append('repeated ' + table.name + ' items = 1')
                print(table2.to_string())
                table_map[schema.schema_name].append(table2)

    table_cache_map = {}
    for name, tables in table_map.items():
        cache = 'package ' + name + ';\n\n'
        for table in tables:
            cache += table.to_string() + '\n'
        table_cache_map[name] = cache
        #print(cache)

    for name, cache in table_cache_map.items():
        proto_name = os.path.join('../protolib/proto', name + '.proto')
        f = open(proto_name, 'w')
        f.write(cache)
        print('gen proto file: %s' % proto_name)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("cmd", help='gen_pb|gen_proto')
    parser.add_argument('-c', '--conf', nargs='+', help='<file1> <file2> ....')
    args = parser.parse_args()

    if args.cmd == 'gen_pb':
        gen_pb()
    elif args.cmd == 'gen_proto':
        gen_proto(args)
    else:
        print('illegal cmd: %s' % args.cmd)


if __name__ == '__main__':
    main()
