# -*- coding: utf-8 -*-
import os
import platform
from loghelper import LogError
from loghelper import LogInfo

# 将PB转换成py格式
def gen_py_from_pb(proto_path):
    files = os.listdir(proto_path)
    for file in files:
        if not file.endswith('.proto'):
            continue
        pre_file = file[:len(file) - len('.proto')]
        pre_file += '_pb2.py'
        pre_file = '../proto/' + pre_file
        if os.path.exists(pre_file):
            pass
        if platform.system() == "Windows":
            command = ".\\bin\\protoc.exe --proto_path=./deploy/proto --python_out=./deploy/proto/ " + file
        else:
            command = "protoc --proto_path=./deploy/proto --python_out=./deploy/proto " + file
        try:
            LogInfo(command)
            os.system(command)
        except BaseException:
            LogError('protoc %s failed' % file)
            raise


