package main

import (
	"common/logger"
	"common/sgl"
	"flag"
	"fmt"
	"new_deploy/deploy"
	"os"
	"strings"
)

var(
	cmd string
	pattern string
	user string
	passwd string
	conf string
	confs []string
)

func argParser() bool {
	if len(os.Args) < 3 {
		logger.Error("too few arguments, please use help")
		return false
	}
	oldArgs := os.Args[0:]
	var newArgs []string
	for idx, item := range os.Args {
		if idx == 1 {
			cmd = item
			continue
		} else if idx == 2 {
			pattern = item
			continue
		}
		newArgs = append(newArgs, item)
	}
	os.Args = newArgs
	flag.StringVar(&user, "user", "", "target machine username")
	flag.StringVar(&passwd, "passwd", "", "target machine password")
	flag.StringVar(&conf, "conf", "", "config dir")
	flag.Parse()
	os.Args = oldArgs

	if cmd == "push_agent" || cmd == "start_agent" {
		pattern = fmt.Sprintf("%s.*.*.*", pattern)
	}

	return true
}

func checkArguments() bool {
	confs = strings.Split(conf, "$")
	if len(confs) == 0 {
		logger.Error("conf parameter can't be empty")
		return false
	}

	// check whether confs are all directory
	for _, cfg := range confs {
		fi, err := os.Stat(cfg)
		if err != nil {
			if !os.IsExist(err) {
				logger.Error("%s is not exist", cfg)
				return false
			}
		} else {
			if !fi.IsDir() {
				logger.Error("%s is not a directory", cfg)
				return false
			}
		}
	}

	patterns := strings.Split(pattern, ".")
	if len(patterns) != 2 && len(patterns) != 4 {
		logger.Error("Use instance pattern like *.*.*.* or *.*")
		return false
	}
	if len(patterns) == 2 {
		pattern = fmt.Sprintf("%s.*.%s.*", patterns[0], patterns[1])
	}
	return true
}

func init()  {
	if !argParser() {
		os.Exit(-1)
	}
	if !checkArguments() {
		os.Exit(-1)
	}
}

func main() {
	singleton := sgl.Singleton{}
	if !singleton.Lock() {
		logger.Fatal("depoly has already runing")
	} else {
		execCmd()
	}
	defer func() {
		logger.Info("finish..................")
		logger.Stop()
		singleton.Unlock()
	}()
}

func execCmd() {
	dep := deploy.Deploy{}
	if !dep.Init(cmd, pattern, user, passwd, confs) {
		logger.Error("deploy.init fail")
		return
	}
	switch cmd {
	case "push_agent":
		dep.PushAgent()
	case "start_agent":
		dep.StartAgent()
	case "start":
		dep.Start()
	case "stop":
		dep.Stop()
	case "restart":
		dep.Restart()
	case "update":
		dep.Update()
	case "check":
		dep.Check()
	case "push":
		dep.Push()
	case "reload":
		dep.Reload()
	case "clean":
		dep.Clean()
	case "copy":
		fmt.Println("......")
	default:
		break
	}
}

