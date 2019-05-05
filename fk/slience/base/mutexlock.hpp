#pragma once

#include "slience/base/config.hpp"

#ifdef M_PLATFORM_WIN
#include "slience/base/win.hpp"
#else 
#include <pthread.h>
#endif

M_BASE_NAMESPACE_BEGIN

class MutexLock {
public:
	MutexLock();

	~MutexLock();

	void lock();

	void unlock();

private:
	MutexLock(const MutexLock&);
	MutexLock& operator=(const MutexLock&);

#ifdef M_PLATFORM_WIN
	CRITICAL_SECTION _cs;

public:
	CRITICAL_SECTION & mutex();

#else
	pthread_mutex_t _mutex;

public:
	pthread_mutex_t & mutex();

#endif
};

class FakeLock {
public:
	FakeLock();

	~FakeLock();

	void lock();

	void unlock();

private:
	FakeLock(const FakeLock&);
	FakeLock& operator=(const FakeLock&);
};

class ScopedLock
{
public:
	ScopedLock(FakeLock&);

	ScopedLock(MutexLock& mutex);
	
	~ScopedLock();

private:
	ScopedLock(const ScopedLock&);
	ScopedLock& operator=(const ScopedLock&);

	MutexLock* _mutex;
};

M_BASE_NAMESPACE_END
