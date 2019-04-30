package sgl

import (
	"syscall"
	"unsafe"
)

var(
	nameMutex = "Sgl_ShaMem"
	kernel = syscall.NewLazyDLL("kernel32.dll")
	mutexId uintptr
)

func lockSgl() (bool) {
	id, _, err := kernel.NewProc("CreateMutexA").Call(0, 1, uintptr(unsafe.Pointer(&nameMutex)))
	mutexId = id
	if err.Error() != "The operation completed successfully." {
		return false
	}
	return true
}

func unlockSgl() {
	syscall.CloseHandle(syscall.Handle(mutexId))
}