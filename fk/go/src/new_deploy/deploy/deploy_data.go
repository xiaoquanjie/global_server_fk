package deploy

import (
	"common/logger"
	"fmt"
	"github.com/golang/protobuf/proto"
	"io/ioutil"
	"new_deploy/proto"
	"os"
	"path"
	"reflect"
)

type ArtifactInstance struct {
	instanceId string
	instanceName string
	artifactName string
	hostName string
	innerIp string
	outerIp string
	deployIp string
	priority int32
}

func (a *ArtifactInstance) String() string {
	return fmt.Sprintf("%s %s", a.instanceId, a.instanceName)
}

type DeployData struct {
	hostMap map[string]protocol.Host
	agentArtifact protocol.AgentArtifact
	templateArtifactMap map[string]protocol.TemplateArtifactItem
	artifactMap map[string]protocol.NormalArtifactItem
	artifactGroupMap map[string]protocol.ArtifactGroupItem
	deployInfo protocol.DeployInfo
	variableMap map[string]string
	worldMap map[string]protocol.World
	artifactInstances []ArtifactInstance
}

func (d *DeployData) init(confs []string) bool {
	d.hostMap = make(map[string]protocol.Host)
	d.templateArtifactMap = make(map[string]protocol.TemplateArtifactItem)
	d.artifactMap = make(map[string]protocol.NormalArtifactItem)
	d.artifactGroupMap = make(map[string]protocol.ArtifactGroupItem)
	d.variableMap = make(map[string]string)
	d.worldMap = make(map[string]protocol.World)

	for _, conf := range confs {
		if !d.loadConf(conf) {
			logger.Error("")
			return false
		}
	}
	return true
}

func (d *DeployData) loadConf(conf string) bool {
	// parse hosts.conf
	hostPath := path.Join(conf, "hosts.conf")
	hostCfg := protocol.HostCfg{}
	if !d.loadProtobuf(&hostCfg, hostPath) {
		return false
	}

	for _, host := range hostCfg.Hosts {
		d.hostMap[host.Name] = *host
	}

	// parse agent_artifacts.conf
	agentArtifactPath := path.Join(conf, "agent_artifacts.conf")
	agentArtifactCfg := protocol.AgentArtifact{}
	if !d.loadProtobuf(&agentArtifactCfg, agentArtifactPath) {
		return false
	}

	d.agentArtifact = agentArtifactCfg

	// parse artifact_templates.conf
	templateArtifactPath := path.Join(conf, "artifact_templates.conf")
	templateArtifactCfg := protocol.TemplateArtifact{}
	if !d.loadProtobuf(&templateArtifactCfg, templateArtifactPath) {
		return false
	}

	for _, item := range templateArtifactCfg.TemplateArtifacts {
		d.templateArtifactMap[item.TemplateName] = *item
	}

	// parse artifacts.conf
	artifactPath := path.Join(conf, "artifacts.conf")
	artifactCfg := protocol.NormalArtifact{}
	if !d.loadProtobuf(&artifactCfg, artifactPath) {
		return false
	}

	for _, item := range artifactCfg.Artifacts {
		d.artifactMap[item.Name] = *item
	}

	// parse artifact_groups.conf
	artifactGroupsPath := path.Join(conf, "artifact_groups.conf")
	artifactGroupCfg := protocol.ArtifactGroup{}
	if !d.loadProtobuf(&artifactGroupCfg, artifactGroupsPath) {
		return false
	}

	for _, item := range artifactGroupCfg.ArtifactGroups {
		d.artifactGroupMap[item.Name] = *item
	}

	// parse deploy.conf
	deployPath := path.Join(conf, "deploy.conf")
	deployCfg := protocol.Deploy{}
	if !d.loadProtobuf(&deployCfg, deployPath) {
		return false
	}

	//  parse global const variable
	d.deployInfo.TmpRootPath = deployCfg.TmpRootPath
	d.deployInfo.DstRootPath = deployCfg.DstRootPath
	d.deployInfo.ListenPort = deployCfg.ListenPort

	for _, item := range deployCfg.Worlds {
		_, exist := d.worldMap[item.Name]
		if exist {
			logger.Error("world name: %s is duplicated", item.Name)
		} else {
			d.worldMap[item.Name] = *item
		}
	}

	return true
}

func (d *DeployData) loadProtobuf(msg proto.Message, conf string) bool {
	file, err := os.Open(conf)
	if err != nil {
		logger.Error("fail to open file: %s", conf)
		return false
	}
	defer file.Close()

	var content []byte
	content, err = ioutil.ReadAll(file)
	if err != nil {
		panic(err)
		return false
	}

	str := string(content[:])
	err = proto.UnmarshalText(str, msg)
	if err != nil {
		logger.Error("fail to parse file: %s for %s", conf, reflect.TypeOf(msg))
		return false
	}
	return true
}