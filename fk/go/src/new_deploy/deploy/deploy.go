package deploy

import (
	"common/logger"
	"fmt"
)

type Deploy struct {
	cmd string
	pattern string
	user string
	passwd string
	confs []string
	deployData DeployData
}

func (d *Deploy) Init(cmd string, pattern string, user string, passwd string, confs []string) bool {
	d.cmd = cmd
	d.pattern = pattern
	d.user = user
	d.passwd = passwd
	d.confs = confs
	if !d.deployData.init(confs) {
		logger.Error("deploydata.init fail")
		return false
	}
	return true
}

func (d *Deploy) PushAgent() {
	fmt.Println(fmt.Sprintf("\n\n==================================== push_agent %s ====================================", d.pattern))
	d.doCmd("push_agent")
}

func (d * Deploy) StartAgent() {
	fmt.Println(fmt.Sprintf("\n\n==================================== start_agent %s ====================================", d.pattern))
	d.doCmd("start_agent")
}

func (d *Deploy) Start() {
	fmt.Println(fmt.Sprintf("\n\n==================================== start %s ==========================================", d.pattern))
	d.doCmd("start")
}

func (d *Deploy) Stop() {
	fmt.Println(fmt.Sprintf("\n\n==================================== stop %s ===========================================", d.pattern))
	d.doCmd("stop")
}

func (d *Deploy) Restart() {
	d.Stop()
	d.Start()
}

func (d *Deploy) Update() {
	d.Stop()
	d.Clean()
	d.Push()
	d.Start()
}

func (d *Deploy) Check() {
	fmt.Println(fmt.Sprintf("\n\n==================================== check %s ==========================================", d.pattern))
	d.doCmd("check")
}

func (d *Deploy) Push() {
	fmt.Println(fmt.Sprintf("\n\n==================================== push %s ==========================================", d.pattern))
	d.doCmd("push")
}

func (d *Deploy) Reload() {
	fmt.Println(fmt.Sprintf("\n\n==================================== reload %s =========================================", d.pattern))
	d.doCmd("reload")
}

func (d *Deploy) Clean() {
	fmt.Println(fmt.Sprintf("\n\n==================================== clean %s ==========================================", d.pattern))
	d.doCmd("clean")
}

func (d *Deploy) doCmd(cmd string) {

}