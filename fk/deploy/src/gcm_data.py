# -*- coding: utf-8 -*-
# 需要安装protobuf
import gen_py_from_pb
import sys
import os
gen_py_from_pb.gen_py_from_pb('./deploy/proto/')
sys.path.append(os.path.join(os.path.dirname(os.path.dirname(__file__)), "./proto/"))
from google.protobuf import text_format
from gcm_pb2 import *
from loghelper import LogError


class ArtifactInstance:
    def __init__(self):
        self.instance_id = None                 # instance的地址。比如：5.1.7.2
        self.instance_name = None               # instance的名称地址。比如：test.zone_1.logic_svr.2
        self.artifact_name = None               # instance对应的artifact名称。比如：logic_svr
        self.host_name = None                   # instance将被部署的目标服务器名。比如：server1
        self.inner_ip = None                    # instance将被部署的目标服务器的内网IP。比如：192.168.1.100
        self.outer_ip = None                    # instance将被部署的目标服务器的外网IP。比如：100.233.66.39
        self.deploy_ip = None                   # instance将被部署的目标服务器的部署IP。比如：192.168.1.100
        self.start_priority = 0                 # instance的启动优先级，默认为0。值越大，越先启动。
        # self.__emergence_order = None           # instance在deploy.conf中出现的顺序。（用于确定启动顺序）
        self.artifact = None
        self.index = 0

    def __str__(self):
        return self.instance_id + " " + self.instance_name


class GcmData:
    def __init__(self):
        self.host_map = {}
        self.agent_artifact = None
        self.template_artifact_map = {}
        self.artifact_map = {}
        self.artifact_group_map = {}
        self.deploy_info = DeployInfo()
        self.variable_map = {}
        self.world_map = {}
        self.artifact_instances = []

    def init(self, conf_paths):
        for path in conf_paths:
            self._load_conf(path)
        # check syntax
        self._check_syntax()
        self._convert_template_artifacts()
        self._gen_instance()

    def _load_conf(self, conf_path):
        # parse hosts.conf
        host_cfg = HostCfg()
        host_path = os.path.join(conf_path, 'hosts.conf')
        self._load_proto(host_path, host_cfg)
        for host in host_cfg.hosts:
            self.host_map[host.name] = host

        # parse agent_artifacts.conf
        agent_artifact = AgentArtifact()
        agent_artifact_path = os.path.join(conf_path, 'agent_artifacts.conf')
        self._load_proto(agent_artifact_path, agent_artifact)
        self.agent_artifact = agent_artifact

        # parse artifact_templates.conf
        template_artifact = TemplateArtifact()
        artifact_templates_path = os.path.join(conf_path, 'artifact_templates.conf')
        self._load_proto(artifact_templates_path, template_artifact)
        for item in template_artifact.template_artifacts:
            self.template_artifact_map[item.template_name] = item

        # parse artifacts.conf
        normal_artifact = NormalArtifact()
        artifact_path = os.path.join(conf_path, 'artifacts.conf')
        self._load_proto(artifact_path, normal_artifact)
        for item in normal_artifact.artifacts:
            self.artifact_map[item.name] = item

        # parse artifact_groups.conf
        artifact_group = ArtifactGroup()
        artifact_groups_path = os.path.join(conf_path, 'artifact_groups.conf')
        self._load_proto(artifact_groups_path, artifact_group)
        for item in artifact_group.artifact_groups:
            self.artifact_group_map[item.name] = item

        # parse deploy.conf
        deploy_path = os.path.join(conf_path, 'deploy.conf')
        deploy = Deploy()
        self._load_proto(deploy_path, deploy)
        for var in deploy.variables:
            self.variable_map[var.key] = var.value

        # parse global const variable
        self.deploy_info.tmp_root_path = deploy.tmp_root_path
        self.deploy_info.dst_root_path = deploy.dst_root_path
        self.deploy_info.listen_port = deploy.listen_port

        for word in deploy.worlds:
            if word.name in self.world_map:
                LogError('world name: %s is duplicated' % word.name)
                raise BaseException()
            else:
                self.world_map[word.name] = word

    @staticmethod
    def _load_proto(file_path, proto):
        if not os.path.exists(file_path):
            return
        with open(file_path) as f:
            text_format.Parse(f.read(), proto)

    def _check_syntax(self):
        function_id_set = set()
        for name, artifact in self.artifact_map.items():
            if artifact.function_id in function_id_set:
                LogError('function_id: %d is duplicated in artifacts.conf' % artifact.function_id)
                raise BaseException()
            function_id_set.add(artifact.function_id)

        world_id_set = set()
        for world_name in self.world_map:
            world = self.world_map[world_name]
            if not world.name:
                LogError('world name is empty in world_id:%d' % world.id)
                raise BaseException()
            if not world.id:
                LogError('world id is empty in world_name:%s' % world.name)
                raise BaseException()
            if not world.user:
                LogError('user is empty in world_name:%s' % world.name)
                raise BaseException()
            if not world.passwd:
                LogError('passwd is empty in world_name:%s' % world.name)
                raise BaseException()
            if world.id in world_id_set:
                LogError('world id: %d is duplicated' % world.id)
                raise BaseException()

            world_id_set.add(world.id)
            zone_name_set = set()
            zone_id_set = set()

            for zone in world.zones:
                if not zone.name:
                    LogError('zone name: is empty in world name: %s' % world.name)
                    raise BaseException()
                if not zone.id:
                    LogError('zone id: is empty in world name: %s' % world.name)
                    raise BaseException()
                if zone.name in zone_name_set:
                    LogError('zone name: %s is duplicated in world name: %s' % (zone.name, world.name))
                    raise BaseException()
                if zone.id in zone_id_set:
                    LogError('zone id: %d is duplicated in world name: %s' % (zone.id, world.name))
                    raise BaseException()

                zone_name_set.add(zone.name)
                zone_id_set.add(zone.id)
                groups_set = []

                for groups in zone.instance_groups:
                    if not groups.artifact_group_name:
                        LogError('artifact_group_name is empty in zone:%s in world:%s' % (zone.name, world_name))
                        raise BaseException()
                    if not groups.host_name:
                        LogError('host_name is empty in zone:%s in world:%s' % (zone.name, world_name))
                        raise BaseException()
                    if not groups.instance_id:
                        LogError('instance_id is empty in zone:%s in world:%s' % (zone.name, world_name))
                        raise BaseException()
                    if [groups.artifact_group_name, groups.instance_id] in groups_set:
                        LogError('group_name: %s, instance_id: %d is duplicated in zone name: %s in world name: %s'
                                 % (groups.artifact_group_name, groups.instance_id, zone.name, world.name))
                        raise BaseException()
                    groups_set.append([groups.artifact_group_name, groups.instance_id])

    def _convert_template_artifacts(self):
        for name, artifact in self.artifact_map.items():
            if not artifact.template_name:
                continue
            if artifact.template_name not in self.template_artifact_map:
                LogError("can't find template artifacts name: %s in artifacts name: %s"
                         % (artifact.template_name, name))
                raise BaseException()
            template_artifact = self.template_artifact_map[artifact.template_name]
            artifact.files.MergeFrom(template_artifact.files)

    def _gen_instance(self):
        for world_name, world in self.world_map.items():
            for zone in world.zones:
                zone_name = zone.name
                for inst_groups in zone.instance_groups:
                    group_name = inst_groups.artifact_group_name
                    if group_name not in self.artifact_group_map:
                        pformat = "can't find artifact_group_name: %s in zone_name:%s in world_name:%s in artifact_groups.conf"
                        LogError(pformat % (group_name, zone_name, world_name))
                        raise BaseException()
                    if inst_groups.host_name not in self.host_map:
                        pformat = "can't find host_name: %s in zone_name:%s in world_name:%s in hosts.conf"
                        LogError(pformat % (inst_groups.host_name, zone_name, world_name))
                        raise BaseException()
                    group = self.artifact_group_map[group_name]
                    for artifact_name in group.artifact_names:
                        if artifact_name not in self.artifact_map:
                            LogError("can't find artifact_group:%s's artifact_name:%s in artifacts.conf"
                                     % (group_name, artifact_name))
                            raise BaseException()
                        artifact = self.artifact_map[artifact_name]
                        host = self.host_map[inst_groups.host_name]
                        instance = ArtifactInstance()
                        instance.instance_id = '%d.%d.%d.%d' % (world.id, zone.id, artifact.function_id, inst_groups.instance_id)
                        instance.instance_name = '%s.%s.%s.%s' % (world_name, zone_name, artifact.name, inst_groups.instance_id)
                        instance.artifact_name = artifact_name
                        instance.host_name = inst_groups.host_name
                        instance.inner_ip = host.inner_ip
                        instance.outer_ip = host.outer_ip
                        instance.deploy_ip = host.deploy_ip
                        instance.start_priority = artifact.start_priority
                        self.artifact_instances.append(instance)



