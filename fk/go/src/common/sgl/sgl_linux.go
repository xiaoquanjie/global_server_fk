package sgl

import (
	"os"
	"syscall"
)

var(
	nameMutex = "Sgl_ShaMem"
	pathDeploy = "./_deploy_.tmp"
	file *os.File
)

const (
	IPC_RMID   = 0
	IPC_CREAT  = 00001000
	IPC_EXCL   = 00002000
	IPC_NOWAIT = 00004000
)

func lockSgl() (bool) {
	flag := os.O_RDWR | os.O_CREATE
	f, err := os.OpenFile(pathDeploy, flag, 0666)
	if err != nil {
		return false
	}
	if err = syscall.Flock(int(f.Fd()), syscall.LOCK_EX|syscall.LOCK_NB); err != nil {
		f.Close()
		return false
	}
	file = f
	return true
}

func unlockSgl() {
	syscall.Flock(int(file.Fd()), syscall.LOCK_UN)
	file.Close()
	os.Remove(pathDeploy)
}
