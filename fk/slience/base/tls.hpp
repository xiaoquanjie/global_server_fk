/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/11/11
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#pragma once

#include "slience/base/config.hpp"
#include <assert.h>
#include "slience/base/win.hpp"
M_BASE_NAMESPACE_BEGIN

#ifdef M_PLATFORM_WIN

template<typename T, int N = 0>
class tlsdata
{
public:
	struct _init_{
		DWORD _tkey;
		_init_(){
			// 创建
			_tkey = TlsAlloc();
			assert(_tkey != TLS_OUT_OF_INDEXES);
		}
		~_init_(){
			T* pv = (T*)TlsGetValue(_tkey);
			TlsFree(_tkey);
			delete pv;
		}
	};

	inline static T& data(){
		T* pv = 0;
		if (0 == (pv = (T*)TlsGetValue(_data._tkey))){
			pv = new T;
			TlsSetValue(_data._tkey, (void*)pv);
		}
		return *pv;
	}

protected:
	tlsdata(const tlsdata&);
	tlsdata& operator=(const tlsdata&);

private:
	static _init_ _data;
};
#else
#include <pthread.h>
template<typename T, int N = 0>
class tlsdata
{
public:
	struct _init_{
		pthread_key_t _tkey;
		_init_(){
			// 创建
			pthread_key_create(&_tkey, destructor);
		}
		~_init_(){
		}
	};

	inline static T& data(){
		T* pv = 0;
		if (0 == (pv = (T*)pthread_getspecific(_data._tkey))){
			pv = new T;
			pthread_setspecific(_data._tkey, (void*)pv);
		}
		return *pv;
	}

protected:
	tlsdata(const tlsdata&);
	tlsdata& operator=(const tlsdata&);
	static void destructor(void* v) {
		T* pv = (T*)v;
		delete pv;
	}

private:
	static _init_ _data;
};

#endif

template<typename T, int N>
typename tlsdata<T, N>::_init_ tlsdata<T, N>::_data;

M_BASE_NAMESPACE_END
