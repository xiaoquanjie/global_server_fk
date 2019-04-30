# -*- coding: utf-8 -*-

from loghelper import LogInfo
from loghelper import LogError
from gcm_data import GcmData
import os
import util
import ssh
import time


class DeployFinishStatus:
    def __init__(self):
        self.show = False
        self.finish = False


class Gcm:
    def __init__(self, user, password, conf_paths, src_path):
        self.user = user
        self.password = password
        self.conf_paths = conf_paths
        self.src_path = src_path
        self.gcm_data = GcmData()
        self.gcm_data.init(self.conf_paths)

    def push_agent(self, pattern):
        LogInfo('\n\n==================================== push_agent %s ====================================' % pattern)
        self.do_cmd('push_agent', pattern)

    def start_agent(self, pattern):
        LogInfo('\n\n==================================== start_agent %s ====================================' % pattern)
        self.do_cmd('start_agent', pattern)

    def start(self, pattern):
        LogInfo('\n\n==================================== start %s ====================================' % pattern)
        self.do_cmd('start', pattern)

    def stop(self, pattern):
        LogInfo('\n\n==================================== stop %s ====================================' % pattern)
        self.do_cmd('stop', pattern)

    def restart(self, pattern):
        self.stop(pattern)
        self.start(pattern)

    def update(self, pattern):
        self.stop(pattern)
        self.push(pattern)
        self.start(pattern)

    def check(self, pattern):
        LogInfo('\n\n==================================== check %s ====================================' % pattern)
        self.do_cmd("check", pattern)

    def push(self, pattern):
        LogInfo('\n\n==================================== push %s ====================================' % pattern)
        self.do_cmd("push", pattern)

    def reload(self, pattern):
        LogInfo('\n\n==================================== reload %s ====================================' % pattern)
        self.do_cmd("reload", pattern)

    def clean(self, pattern):
        LogInfo('\n\n==================================== clean %s ====================================' % pattern)
        self.do_cmd('clean', pattern)

    def copy(self, source_host, host_list):
        LogInfo('\n\n==================================== copy %s ====================================' % host_list)
        pass

    def do_cmd(self, cmd, pattern):
        # get instances
        instance_list = util.get_instances(self.gcm_data.artifact_instances, pattern)
        if not instance_list:
            LogError('find no instance in pattern: %s' % pattern)
            return

        # get host set
        host_set = set()
        for instance in instance_list:
            host_set.add(instance.deploy_ip)

        # get user and passwd
        self._get_user_and_passwd(instance_list)

        # do real jobs
        if cmd == 'push_agent':
            self._push_agent(instance_list, list(host_set), pattern)
        elif cmd == 'start_agent':
            self._start_agent(instance_list, list(host_set), pattern)
        elif cmd == 'start':
            self._start(instance_list, list(host_set), pattern)
        elif cmd == 'stop':
            self._stop(instance_list, list(host_set), pattern)
        elif cmd == 'check':
            self._check(instance_list, list(host_set), pattern)
        elif cmd == 'push':
            self._push(instance_list, list(host_set), pattern)
        elif cmd == 'reload':
            self._reload(instance_list, list(host_set), pattern)
        elif cmd == 'clean':
            self._clean(instance_list, list(host_set), pattern)

    def _push_agent(self, instance_list, host_list, pattern):
        agent_repo = util.pack_agent_files(
                                     self.gcm_data.deploy_info.tmp_root_path,
                                     self.src_path,
                                     self.gcm_data.agent_artifact.files,
                                     util.get_agent_repo_name())
        dst_root_path = self.gcm_data.deploy_info.dst_root_path
        remote_agent_repo = util.get_remote_agent_repo(dst_root_path)
        for host in host_list:
            self._real_push(host, agent_repo, remote_agent_repo)
            cmd = 'cd ' + self.gcm_data.deploy_info.dst_root_path
            cmd += '; chmod +x ./repo/deploy/bin/*'
            ssh.ssh_cmd(host, self.user, self.password, cmd)

    def _start_agent(self, instance_list, host_list, pattern):
        pass

    def _start(self, instance_list, host_list, pattern):
        pass

    def _stop(self, instance_list, host_list, pattern):
        pass

    def _check(self, instance_list, host_list, pattern):
        pass

    def _push(self, instance_list, host_list, pattern):
        # check all machine whether has agent
        finish_host_status = {}
        for host in host_list:
            finish_host_status[host] = DeployFinishStatus()
            cmd = 'test -e ' + util.get_remote_deploy_file(self.gcm_data.deploy_info.dst_root_path)
            ssh.ssh_cmd(host, self.user, self.password, cmd)

        pack_repo = util.pack_repo_files(self.gcm_data.artifact_map,
                                           self.gcm_data.deploy_info.tmp_root_path,
                                           self.src_path, instance_list,
                                           util.get_repo_name())
        dst_root_path = self.gcm_data.deploy_info.dst_root_path
        remote_repo = util.get_remote_repo(dst_root_path)
        self._real_push(host_list[0], pack_repo, remote_repo)

        # wait
        finish_host_status[host_list[0]].finish = True
        while True:
            finish_cnt = 0
            for key, value in finish_host_status.items():
                if value.finish:
                    finish_cnt += 1
                    if not value.show:
                        LogInfo('host: %s is finish' % key)
                        value.show = True
            if finish_cnt == len(finish_host_status):
                break
            time.sleep(1)

        LogInfo('success to finish')

    def _reload(self, instance_list, host_list, pattern):
        pass

    def _clean(self, instance_list, host_list, pattern):
        pass

    def _get_user_and_passwd(self, instance_list):
        if not self.user:
            if not instance_list:
                return
            world_name = instance_list[0].instance_name
            world_name = world_name.split('.')[0]
            world = self.gcm_data.world_map[world_name]
            self.user = world.user
            self.password = world.passwd

    def _real_push(self, host, pack_repo, remote_pack_repo):
        # upload
        ssh.ssh_upload_repo(host,
                            self.user,
                            self.password,
                            pack_repo,
                            remote_pack_repo)
        # unpack
        cmd = 'cd ' + os.path.dirname(remote_pack_repo);
        cmd += '; tar -zxvf '
        cmd += os.path.basename(remote_pack_repo)
        return ssh.ssh_cmd(
            host,
            self.user,
            self.password,
            cmd)
