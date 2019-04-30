# -*- coding: utf-8 -*-
import tarfile
import re
import os
import loghelper


def regex_pattern(pattern):
    return "^" + "\\.".join([s if s != "*" else ".+" for s in pattern.split(".")]) + "$"


def get_instances(artifact_instances, pattern):
    def priority(instance):
        return instance.start_priority

    reg_pattern = regex_pattern(pattern)
    instance_list = []
    for instance in artifact_instances:
        if not re.match(reg_pattern, instance.instance_name):
            continue
        instance_list.append(instance)

    instance_list.sort(key=priority, reverse=True)
    return instance_list


def _pack_files_begin(tmp_path, src_path, dstfile):
    loghelper.LogInfo(
        '\n\n==================================== collecting file or directory ====================================')
    if not os.path.exists(src_path):
        loghelper.LogError('src_path: %s is not exist' % src_path)
        raise BaseException()
    if not os.path.exists(tmp_path):
        os.mkdir(tmp_path)

    pack_path = os.path.join(tmp_path, dstfile)
    tar_file = tarfile.open(pack_path, "w:gz")
    cwd = os.getcwd()
    os.chdir(src_path)
    return tar_file, cwd, pack_path


def _pack_files_end(tar_file, cwd, pack_path):
    tar_file.close()
    os.chdir(cwd)
    loghelper.LogInfo('tar.gz file: %s' % pack_path)


def pack_agent_files(tmp_path, src_path, artifacts_files, dstfile):
    tar_file, cwd, pack_path = _pack_files_begin(tmp_path, src_path, dstfile)
    for f in artifacts_files:
        loghelper.LogInfo(f.src)
        tar_file.add(f.src)
    _pack_files_end(tar_file, cwd, pack_path)
    return pack_path


def pack_repo_files(artifact_map, tmp_path, src_path, instance_list, dstfile):
    tar_file, cwd, pack_path = _pack_files_begin(tmp_path, src_path, dstfile)
    for instance in instance_list:
        artifacts = artifact_map[instance.artifact_name]
        for f in artifacts.files:
            loghelper.LogInfo(f.src)
            tar_file.add(f.src)
    _pack_files_end(tar_file, cwd, pack_path)
    return pack_path


def get_repo_name():
    return 'repo.tar.gz'


def get_agent_repo_name():
    return 'deploy.tar.gz'


def get_remote_repo(dst_root_path):
    remote_repo = os.path.join(dst_root_path, 'repo/')
    remote_repo = os.path.join(remote_repo, get_repo_name())
    return remote_repo


def get_remote_agent_repo(dst_root_path):
    remote_repo = os.path.join(dst_root_path, 'repo/')
    remote_repo = os.path.join(remote_repo, get_agent_repo_name())
    return remote_repo


def get_unpack_remote_repo_cmd(dst_root_path):
    cmd = 'cd ' + os.path.dirname(get_remote_repo(dst_root_path))
    cmd += '; tar -zxvf '
    cmd += get_repo_name()
    return cmd


def get_unpack_remote_agent_cmd(dst_root_path):
    cmd = 'cd ' + os.path.dirname(get_remote_repo(dst_root_path))
    cmd += '; tar -zxvf '
    cmd += get_agent_repo_name()
    return cmd


def get_remote_deploy_file(dst_root_path):
    path = os.path.join(dst_root_path, 'repo/deploy/src/deploy.py')
    return path
