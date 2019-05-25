#pragma once

#include "commonlib/svr_base/svrbase.h"
#include "slience/base/newonce_pool.h"

class TcpSocketMsgAlloc {
public:
	static TcpSocketMsg* Alloc() {
		auto p = base::NewOncePool<TcpSocketMsg>::Alloc();
		p->Clear();
		return p;
	}

	static void Dealloc(TcpSocketMsg* msg) {
		base::NewOncePool<TcpSocketMsg>::Dealloc(msg);
	}

	static int GetCount() {
		return base::NewOncePool<TcpSocketMsg>::GetCount();
	}

	static int GetAllocSize() {
		return base::NewOncePool<TcpSocketMsg>::GetAllocSize();
	}

	static int GetUsingCount() {
		return base::NewOncePool<TcpSocketMsg>::GetUsingCount();
	}
};

class TcpConnectorMsgAlloc {
public:
	static TcpConnectorMsg* Alloc() {
		auto p = base::NewOncePool<TcpConnectorMsg>::Alloc();
		p->Clear();
		return p;
	}

	static void Dealloc(TcpConnectorMsg* msg) {
		base::NewOncePool<TcpConnectorMsg>::Dealloc(msg);
	}

	static int GetCount() {
		return base::NewOncePool<TcpConnectorMsg>::GetCount();
	}

	static int GetAllocSize() {
		return base::NewOncePool<TcpConnectorMsg>::GetAllocSize();
	}

	static int GetUsingCount() {
		return base::NewOncePool<TcpConnectorMsg>::GetUsingCount();
	}
};

class SelfMsgAlloc {
public:
	static SelfMsg* Alloc() {
		auto p = base::NewOncePool<SelfMsg, 100>::Alloc();
		return p;
	}

	static void Dealloc(SelfMsg* msg) {
		base::NewOncePool<SelfMsg, 100>::Dealloc(msg);
	}

	static int GetCount() {
		return base::NewOncePool<SelfMsg, 100>::GetCount();
	}

	static int GetAllocSize() {
		return base::NewOncePool<SelfMsg, 100>::GetAllocSize();
	}

	static int GetUsingCount() {
		return base::NewOncePool<SelfMsg, 100>::GetUsingCount();
	}
};