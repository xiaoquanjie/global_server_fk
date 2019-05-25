#pragma once

#include <string.h>
#include <stdlib.h>
#include "slience/base/config.hpp"
#include "slience/base/mutexlock.hpp"

M_BASE_NAMESPACE_BEGIN

// 默认是伪锁（即无锁）
template<typename T, int MaxAllocNum = 10000, typename LockMode = base::FakeLock>
class NewOncePool {
protected:
	struct ObjectInfo {
		int _numalloc;
		int _numusing;
		size_t _elemsize;
		T*_listhead;
		LockMode _lock;

		ObjectInfo() {
			_numalloc = 0;
			_numusing = 0;
			_elemsize = sizeof(T) + sizeof(T*);
			_listhead = 0;
		}

		~ObjectInfo() {
			while (_listhead) {
				char* ret = (char*)_listhead;
				_listhead = *(reinterpret_cast<T**>(_listhead));
				T* obj = (T*)(ret + sizeof(T*));
				obj->~T();
				free(ret);
			}
		}
	};
	static ObjectInfo _info;

public:
	static int GetCount() {
		return _info._numalloc;
	}

	static int GetAllocSize() {
		return _info._numalloc *_info._elemsize;
	}

	static int GetUsingCount() {
		return _info._numusing;
	}

	static T* Alloc() {
		base::ScopedLock lock(_info._lock);
		T* obj = 0;
		if (_info._listhead == NULL) {
			char* ret = (char*)malloc(_info._elemsize);
			obj = (T*)(ret + sizeof(T*));
			new(obj)T();
			_info._numalloc++;
		}
		else {
			char* ret = (char*)_info._listhead;
			_info._listhead = *(reinterpret_cast<T**>(_info._listhead));
			obj = (T*)(ret + sizeof(T*));
		}
		_info._numusing++;
		return obj;
	}

	static void Dealloc(T* elem) {
		base::ScopedLock lock(_info._lock);
		char* ori = (char*)elem - sizeof(T*);
		if (_info._numalloc > MaxAllocNum) {
			_info._numalloc--;
			elem->~T();
			free(ori);
		}
		else {
			*(reinterpret_cast<T**>(ori)) = _info._listhead;
			_info._listhead = (T*)ori;
		}
		_info._numusing--;
	}
};

template<typename T, int MaxAllocNum, typename LockMode>
typename NewOncePool<T, MaxAllocNum, LockMode>::ObjectInfo
NewOncePool<T, MaxAllocNum, LockMode>::_info;

M_BASE_NAMESPACE_END