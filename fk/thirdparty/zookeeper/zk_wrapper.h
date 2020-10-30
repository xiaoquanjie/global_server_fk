#pragma once

#include "zookeeper.h"
#include <memory>
#include <unordered_map>
#include <string>
#include "objectpool.h"

class ZkConnMgr;
class ZkBaseConnection;

#ifndef WIN32
typedef int SOCKET;
#endif

struct ZkSelectCtxt {
	zhandle_t* handler;
	SOCKET fd;
};

struct ZkConnectCtxt {
	ZkConnMgr* mgr;
	std::shared_ptr<ZkBaseConnection> ptr;
};

struct ZkRequestContext {
	std::shared_ptr<ZkBaseConnection> ptr;
	std::string path;
	std::string op;
};

////////////////////////////////////////////////////////////////////////

class ZkBaseConnection : public std::enable_shared_from_this<ZkBaseConnection> {
	friend class ZkConnMgr;
public:
	ZkBaseConnection();

	virtual ~ZkBaseConnection();

	bool IsConnected() const;

	// @acl: ZOO_OPEN_ACL_UNSAFE etc
	// @flags: ZOO_EPHEMERAL,ZOO_SEQUENCE
	int acreate(const char *path,
		const char *value, int valuelen,
		const struct ACL_vector *acl, int flags);

	// Ĭ��ʹ��digestģʽ
	int acreate(const char *path, const char *value, int valuelen, const char* user,
		const char* passwd, int flags);

	int adelete(const char *path, int version);

	int aexists(const char *path, int watch);

	int aget(const char *path, int watch);

	int aset(const char *path, const char *buffer, int buflen, int version);

	int aget_children(const char *path, int watch);

	int aget_acl(const char* path);

	int aset_acl(const char* path, int version, struct ACL_vector *acl);

	int aset_acl(const char* path, int version, const char* user,
		const char* passwd);

	// �������ζ�������Ͽ�����
	void close();

protected:
	// a node has been created.
	virtual void OnCreateNode(const char* path) {}

	// a node has been deleted.
	virtual void OnDeletedNode(const char* path) {}

	// a node has changed.
	virtual void OnChangedNode(const char* path) {}

	// a change as occurred in the list of children.
	virtual void OnChild(const char* path) {}

	virtual void OnSeesion(int state, const char* path) {}

	////////////////////////////////////////////////////////////////////////////////////////

	// ���� Stat �ṹ�Ļص�����
	virtual void OnVoidCompletionCb(int rc, const char* op, const char* path) {}

	// ���� Stat �ṹ�Ļص�����
	virtual void OnStatCompletionCb(int rc, const char* op, const char* path, const struct Stat *stat) {}

	// �����ַ����Ļص�����
	virtual void OnStringCompletionCb(int rc, const char* op, const char* path, const char *value) {}

	// �������ݵĻص�����
	virtual void OnDataCompletionCb(int rc, const char* op, const char* path, const char *value, int value_len, const struct Stat *stat){}

	// �����ַ����б�(a list of string)�Ļص�����
	virtual void OnStringsCompletionCb(int rc, const char* op, const char* path, const struct String_vector *strings) {}

	// ͬʱ�����ַ����б�(a list of string)�� Stat �ṹ�Ļص�����
	virtual void OnStringsStatCompletionCb(int rc, const char* op, const char* path, const struct String_vector *strings,
		const struct Stat *stat) {}

	// �Լ����� ACL ��Ϣ�Ļص�����
	virtual void OnAclCompletionCb(int rc, const char* op, const char* path, struct ACL_vector *acl, struct Stat *stat) {}

private:
	zhandle_t* handler;
	bool connected;
	std::string host;
	int timeout;
	int flags;
};

/////////////////////////////////////////////////////////////////////////////////////////////

class ZkConnMgr {
	friend class ZkBaseConnection;

public:
	ZkConnMgr();

	~ZkConnMgr();

	// return is idle
	bool update();

	// ֻ��������64��(windows)
	template<typename T>
	int connect(const char *host, int timeout, int flags = 0);

	static void SetLog(ZooLogLevel lvl);

private:
	template<typename T>
	static void watcher_cb(zhandle_t* zh, int type, int state,
		const char* path, void* watcherCtx);

	// ���� void ���͵Ļص�����
	static void void_completion_cb(int rc, const void *data);

	// ���� Stat �ṹ�Ļص�����
	static void stat_completion_cb(int rc, const struct Stat *stat, const void *data);

	// �����ַ����Ļص�����
	static void string_completion_cb(int rc, const char *value, const void *data);

	// �������ݵĻص�����
	static void data_completion_cb(int rc, const char *value, int value_len, const struct Stat *stat, const void *data);

	// �����ַ����б�(a list of string)�Ļص�����
	static void strings_completion_cb(int rc, const struct String_vector *strings, const void *data);

	// ͬʱ�����ַ����б�(a list of string)�� Stat �ṹ�Ļص�����
	static void strings_stat_completion_cb(int rc, const struct String_vector *strings, const struct Stat *stat, const void *data);

	// �Լ����� ACL ��Ϣ�Ļص�����
	static void acl_completion_cb(int rc, struct ACL_vector *acl, struct Stat *stat, const void *data);

private:
	std::unordered_map<zhandle_t*, std::shared_ptr<ZkBaseConnection>> container;
};

////////////////////////////////////////////////////////////////////////////

template<typename T>
int ZkConnMgr::connect(const char *host, int timeout, int flags) {
	if (container.size() >= FD_SETSIZE) {
		return -1;
	}

	auto ptr = std::make_shared<T>();
	ptr->timeout = timeout;
	ptr->host = host;
	ptr->flags = flags;
	ptr->connected = false;

	ZkConnectCtxt* c_ctxt = new ZkConnectCtxt;
	c_ctxt->ptr = ptr;
	c_ctxt->mgr = this;

	ptr->handler = zookeeper_init(host, &ZkConnMgr::watcher_cb<T>, timeout, 0, (void*)c_ctxt, flags);
	if (ptr->handler == NULL) {
		delete c_ctxt;
		return -1;
	}

	container.insert(std::make_pair(ptr->handler, ptr));
	return 0;
}

template<typename T>
void ZkConnMgr::watcher_cb(zhandle_t* zh, int type, int state,
	const char* path, void* watcherCtx) {
	ZkConnectCtxt* c_ctxt = (ZkConnectCtxt*)watcherCtx;

	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			c_ctxt->ptr->connected = true;
		}
		else if (state == ZOO_CONNECTING_STATE) {
			c_ctxt->ptr->connected = false;
		}
	}

	if (type == ZOO_SESSION_EVENT) {
		c_ctxt->ptr->OnSeesion(state, path);
	}
	else if (type == ZOO_CREATED_EVENT) {
		c_ctxt->ptr->OnCreateNode(path);
	}
	else if (type == ZOO_DELETED_EVENT) {
		c_ctxt->ptr->OnDeletedNode(path);
	}
	else if (type == ZOO_CHANGED_EVENT) {
		c_ctxt->ptr->OnChangedNode(path);
	}
	else if (type == ZOO_CHILD_EVENT) {
		c_ctxt->ptr->OnChild(path);
	}

	// �Ự�¼�
	if (type == ZOO_SESSION_EVENT
		&& state == ZOO_EXPIRED_SESSION_STATE) {
		// ���������Ͽ�,������
		if (c_ctxt->ptr->handler != 0) {
			c_ctxt->mgr->connect<T>(c_ctxt->ptr->host.c_str(), c_ctxt->ptr->timeout, c_ctxt->ptr->flags);
		}

		// ���
		c_ctxt->mgr->container.erase(c_ctxt->ptr->handler);
		delete c_ctxt;
	}
}