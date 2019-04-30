# -*- coding: utf-8 -*-
import sys
import os


def check_file_exist(dir_path, file_name):
    files = os.listdir(dir_path)
    for file in files:
        if file == file_name:
            return True
    return False


def get_deploy_root_exist():
    exec_path = os.path.split(os.path.realpath(__file__))[0]
    exe_dir = os.path.dirname(exec_path)
    deploy_root = None
    while True:
        if check_file_exist(exe_dir, 'DEPLOY_ROOT'):
            deploy_root = exe_dir
            break
        new_exe_dir = os.path.dirname(exe_dir)
        if new_exe_dir == exe_dir:
            break
        exe_dir = new_exe_dir

    if not deploy_root:
        print("can't find the DEPLOY_ROOT file")
        return None
    else:
        print('DEPLOY_ROOT file in: %s' % deploy_root)
        return deploy_root


# 查找出DEPLOY_ROOT文件目录
deploy_root = get_deploy_root_exist()
if not deploy_root:
    sys.exit(-1)
os.chdir(deploy_root)


from singleton_instance import SingletonInstance
import argparse
import loghelper
from gcm import Gcm


def arg_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument("cmd", help='push_agent|start_agent|start|stop|restart|update|check|push|reload|clean')
    parser.add_argument('pattern', help='*.*.*.* or *.* or *')
    parser.add_argument('-u', '--user', help='target machine username')
    parser.add_argument('-p', '--password', help='target machine password')
    parser.add_argument('-c', '--conf', nargs='+', help='config path')
    parser.add_argument('-a', '--address')
    parser.add_argument('-s', '--source')
    return parser.parse_args()


def check_argument(args):
    if not args.conf:
        loghelper.LogError("conf parameter can't be empty")
        return False
    else:
        for conf in args.conf:
            if not os.path.isdir(conf):
                loghelper.LogError('%s is not dir' % conf)
                return False

    if args.cmd in ['push_agent', 'start_agent']:
        args.pattern = '%s.*.*.*' % args.pattern
    elif args.cmd == 'copy':
        if not args.address:
            loghelper.LogError('address is empty')
            return False
    else:
        patterns = args.pattern.split('.')
        if len(patterns) not in [2, 4]:
            loghelper.LogError('Use instance pattern like *.*.*.* or *.*')
            return False
        if len(patterns) == 2:
            args.pattern = "%s.*.%s.*" % (patterns[0], patterns[1])
    return True


def exec_cmd(args, deploy_root):
    try:
        gcm = Gcm(args.user, args.password, args.conf, deploy_root)
        if args.cmd == 'push_agent':
            gcm.push_agent(args.pattern)
        elif args.cmd == 'start_agent':
            gcm.start_agent(args.pattern)
        elif args.cmd == 'start':
            gcm.start(args.pattern)
        elif args.cmd == 'stop':
            gcm.stop(args.pattern)
        elif args.cmd == 'restart':
            gcm.restart(args.pattern)
        elif args.cmd == 'update':
            gcm.update(args.pattern)
        elif args.cmd == 'check':
            gcm.check(args.pattern)
        elif args.cmd == 'push':
            gcm.push(args.pattern)
        elif args.cmd == 'reload':
            gcm.reload(args.pattern)
        elif args.cmd == 'clean':
            gcm.clean(args.pattern)
        elif args.cmd == 'copy':
            gcm.copy(args.source, args.address)
        else:
            loghelper.LogError('illegal cmd: %s' % args.cmd)
    #except BaseException as err:
        #LogError(err)
    finally:
        pass


if __name__ == '__main__':
    singleton = SingletonInstance()
    if singleton.already_running():
        print('Error: A Deploy Program is already running, please try it later')
        sys.exit(-1)

    args = arg_parser()
    if not check_argument(args):
        sys.exit(-1)

    exec_cmd(args, deploy_root)
    sys.exit(0)


