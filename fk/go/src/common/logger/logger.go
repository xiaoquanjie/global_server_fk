package logger

import (
	"github.com/golang/glog"
)

func init() {
}

func Debug(format string, args ... interface{}) {
	glog.Debugf(format, args...)
}

func Info(format string, args ... interface{}) {
	glog.Infof(format, args...)
}

func Warn(format string, args ... interface{})  {
	glog.Warningf(format, args...)
}

func Error(format string, args ... interface{}) {
	glog.Errorf(format, args...)
}

func Fatal(format string, args ... interface{})  {
	glog.Fatalf(format, args...)
}

func Stop()  {
	glog.Exit()
}
