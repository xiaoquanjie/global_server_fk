# -*- coding: utf-8 -*-
import platform

# windows版下载安装pywin32
# https://github.com/mhammond/pywin32/releases
# pip install pywin32

if platform.system() == 'Windows':
    import win32event
    import winerror
    import win32api
else:
    import fcntl


class SingletonInstance:
    def __init__(self):
        if platform.system() == 'Windows':
            mutex_name = '_deploy_mutext_2018_'
            self.mutex = win32event.CreateMutex(None, False, mutex_name)
            self.last_error = win32api.GetLastError()
        else:
            file_path = "./_deploy_.tmp"
            self.file = open(file_path, 'w')
            try:
                fcntl.flock(self.file, fcntl.LOCK_EX | fcntl.LOCK_NB)
                self.last_error = None
            except IOError:
                self.last_error = IOError()

    def already_running(self):
        if platform.system() == 'Windows':
            return self.last_error == winerror.ERROR_ALREADY_EXISTS
        else:
            return self.last_error != None

    def __del__(self):
        if platform.system() == 'Windows':
            if self.mutex:
                win32api.CloseHandle(self.mutex)
        else:
            if self.last_error == None:
                fcntl.flock(self.file, fcntl.LOCK_UN)
            self.file.close()
