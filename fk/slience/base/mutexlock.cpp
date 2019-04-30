#include "slience/base/mutexlock.hpp"
#include <assert.h>

M_BASE_NAMESPACE_BEGIN

#ifdef M_PLATFORM_WIN
MutexLock::MutexLock() {
	InitializeCriticalSection(&_cs);
}

MutexLock::~MutexLock() {
	DeleteCriticalSection(&_cs);
}

void MutexLock::lock() {
	EnterCriticalSection(&_cs);
}

void MutexLock::unlock() {
	LeaveCriticalSection(&_cs);
}

CRITICAL_SECTION& MutexLock::mutex() {
	return _cs;
}

#else
MutexLock::MutexLock() {
	pthread_mutex_init(&_mutex, 0);
}

MutexLock::~MutexLock() {
	pthread_mutex_destroy(&_mutex);
}

void MutexLock::lock() {
	pthread_mutex_lock(&_mutex);
}

void MutexLock::unlock() {
	pthread_mutex_unlock(&_mutex);
}

pthread_mutex_t& MutexLock::mutex() {
	return _mutex;
}

#endif

FakeLock::FakeLock() {
}

FakeLock::~FakeLock() {
}

void FakeLock::lock() {
}

void FakeLock::unlock() {
}

ScopedLock::ScopedLock(FakeLock&) {
	_mutex = 0;
}

ScopedLock::ScopedLock(MutexLock& mutex) {
	_mutex = &mutex;
	_mutex->lock();
}

ScopedLock::~ScopedLock() {
	if (_mutex) {
		_mutex->unlock();
	}
}

M_BASE_NAMESPACE_END