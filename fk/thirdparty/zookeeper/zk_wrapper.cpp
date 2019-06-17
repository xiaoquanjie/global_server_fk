#include "zk_wrapper.h"

ZkBaseConnection::ZkBaseConnection() {
	ctxt = 0;
	handler = 0;
	watcherCtx = 0;
	connected = false;
	timeout = 0;
	flags = 0;
}

ZkBaseConnection::ZkBaseConnection(void* watcherCtx) {
	ctxt = 0;
	handler = 0;
	this->watcherCtx = watcherCtx;
	connected = false;
	timeout = 0;
	flags = 0;
}

ZkBaseConnection::~ZkBaseConnection() {
	close();
}

bool ZkBaseConnection::IsConnected() const {
	return connected;
}

int ZkBaseConnection::acreate(const char *path,
	const char *value, int valuelen,
	const struct ACL_vector *acl, int flags) {
	ZkRequestContext* r_ctxt = this->ctxt->mgr->request_alloc.alloc();
	r_ctxt->ctxt = this->ctxt;
	r_ctxt->path = path;
	r_ctxt->op = "acreate";

	int ret = zoo_acreate(handler, path, value, valuelen, acl, flags, 
		&ZkConnMgr::string_completion_cb, (void*)r_ctxt);
	if (ret != ZOK) {
		this->ctxt->mgr->request_alloc.dealloc(r_ctxt);
	}
	return ret;
}

int ZkBaseConnection::acreate(const char *path, const char *value, int valuelen, const char* user,
	const char* passwd, int flags) {
	if (!user || std::string(user) == std::string("")) {
		return acreate(path, value, valuelen, &ZOO_OPEN_ACL_UNSAFE, flags);
	}
	else {
		struct ACL myacl[1]; //{ {0x1f, {"digest", "123"}} };
		myacl[0].perms = 0x1f;
		myacl[0].id.scheme = const_cast<char*>("digest");
		myacl[0].id.id = const_cast<char*>((std::string(user) + std::string(":") + std::string(passwd)).c_str());

		struct ACL_vector myacl_vec = { 1, myacl };
		return acreate(path, value, valuelen, &myacl_vec, flags);
	}
}

int ZkBaseConnection::adelete(const char *path, int version) {
	ZkRequestContext* r_ctxt = this->ctxt->mgr->request_alloc.alloc();
	r_ctxt->ctxt = this->ctxt;
	r_ctxt->path = path;
	r_ctxt->op = "adelete";

	int ret = zoo_adelete(handler, path, version, &ZkConnMgr::void_completion_cb, (void*)r_ctxt);
	if (ret != ZOK) {
		this->ctxt->mgr->request_alloc.dealloc(r_ctxt);
	}
	return ret;
}

int ZkBaseConnection::aexists(const char *path, int watch) {
	ZkRequestContext* r_ctxt = this->ctxt->mgr->request_alloc.alloc();
	r_ctxt->ctxt = this->ctxt;
	r_ctxt->path = path;
	r_ctxt->op = "aexists";

	int ret = zoo_aexists(handler, path, watch, &ZkConnMgr::stat_completion_cb, (void*)r_ctxt);
	if (ret != ZOK) {
		this->ctxt->mgr->request_alloc.dealloc(r_ctxt);
	}
	return ret;
}

int ZkBaseConnection::aget(const char *path, int watch) {
	ZkRequestContext* r_ctxt = this->ctxt->mgr->request_alloc.alloc();
	r_ctxt->ctxt = this->ctxt;
	r_ctxt->path = path;
	r_ctxt->op = "aget";

	int ret = zoo_aget(handler, path, watch, &ZkConnMgr::data_completion_cb, (void*)r_ctxt);
	if (ret != ZOK) {
		this->ctxt->mgr->request_alloc.dealloc(r_ctxt);
	}
	return ret;
}

int ZkBaseConnection::aset(const char *path, const char *buffer, int buflen, int version) {
	ZkRequestContext* r_ctxt = this->ctxt->mgr->request_alloc.alloc();
	r_ctxt->ctxt = this->ctxt;
	r_ctxt->path = path;
	r_ctxt->op = "aset";

	int ret = zoo_aset(handler, path, buffer, buflen, version, &ZkConnMgr::stat_completion_cb, (void*)r_ctxt);
	if (ret != ZOK) {
		this->ctxt->mgr->request_alloc.dealloc(r_ctxt);
	}
	return ret;
}

int ZkBaseConnection::aget_children(const char *path, int watch) {
	ZkRequestContext* r_ctxt = this->ctxt->mgr->request_alloc.alloc();
	r_ctxt->ctxt = this->ctxt;
	r_ctxt->path = path;
	r_ctxt->op = "aget_children";

	int ret = zoo_aget_children(handler, path, watch, &ZkConnMgr::strings_completion_cb, (void*)r_ctxt);
	if (ret != ZOK) {
		this->ctxt->mgr->request_alloc.dealloc(r_ctxt);
	}
	return ret;
}

int ZkBaseConnection::aget_acl(const char* path) {
	ZkRequestContext* r_ctxt = this->ctxt->mgr->request_alloc.alloc();
	r_ctxt->ctxt = this->ctxt;
	r_ctxt->path = path;
	r_ctxt->op = "aget_acl";
	int ret = zoo_aget_acl(handler, path, &ZkConnMgr::acl_completion_cb, (void*)r_ctxt);
	if (ret != ZOK) {
		this->ctxt->mgr->request_alloc.dealloc(r_ctxt);
	}
	return ret;
}

int ZkBaseConnection::aset_acl(const char* path, int version, struct ACL_vector *acl) {
	ZkRequestContext* r_ctxt = this->ctxt->mgr->request_alloc.alloc();
	r_ctxt->ctxt = this->ctxt;
	r_ctxt->path = path;
	r_ctxt->op = "aset_acl";
	int ret = zoo_aset_acl(handler, path, version, acl, &ZkConnMgr::void_completion_cb, (void*)r_ctxt);
	if (ret != ZOK) {
		this->ctxt->mgr->request_alloc.dealloc(r_ctxt);
	}
	return ret;
}

int ZkBaseConnection::aset_acl(const char* path, int version, const char* user,
	const char* passwd) {
	if (!user || std::string(user) == std::string("")) {
		return aset_acl(path, version, &ZOO_OPEN_ACL_UNSAFE);
	}
	else {
		struct ACL myacl[1]; //{ {0x1f, {"digest", "123"}} };
		myacl[0].perms = 0x1f;
		myacl[0].id.scheme = const_cast<char*>("digest");
		myacl[0].id.id = const_cast<char*>((std::string(user) + std::string(":") + std::string(passwd)).c_str());

		struct ACL_vector myacl_vec = { 1, myacl };
		return aset_acl(path, version, &myacl_vec);
	}
}

void ZkBaseConnection::close() {
	if (handler) {
		zookeeper_close(handler);
		handler = 0;
		connected = false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

ZkConnMgr::ZkConnMgr() {

}

ZkConnMgr::~ZkConnMgr() {
	for (auto iter = container.begin(); iter != container.end(); ++iter) {
		iter->second->ptr->close();
		delete iter->second;
	}
	container.clear();
}

int ZkConnMgr::update() {
	ZkSelectCtxt ctxt[FD_SETSIZE];
	int idx = 0;
	unsigned int maxfd = 0;
	struct timeval wait_tv = {0, 10};
	fd_set r_fd_set;
	fd_set w_fd_set;
	fd_set e_fd_set;
	FD_ZERO(&r_fd_set);
	FD_ZERO(&w_fd_set);
	FD_ZERO(&e_fd_set);
	
	for (auto iter = container.begin(); iter != container.end();) {
		const std::shared_ptr<ZkBaseConnection>& conn = iter->second->ptr;
		if (conn->handler == 0) {
			iter = container.erase(iter);
		}
		else {
			SOCKET fd = 0;
			int interest = 0;
			struct timeval tv;
			zookeeper_interest(conn->handler, &fd, &interest, &tv);
			if (fd != -1) {
				if (interest & ZOOKEEPER_READ) {
					FD_SET(fd, &r_fd_set);
				}
				else {
					FD_CLR(fd, &r_fd_set);
				}

				if (interest & ZOOKEEPER_WRITE) {
					FD_SET(fd, &w_fd_set);
				}
				else {
					FD_CLR(fd, &w_fd_set);
				}
				if (fd > maxfd) {
					maxfd = fd;
				}
				ctxt[idx].fd = fd;
				ctxt[idx].handler = conn->handler;
				idx++;
			}
			++iter;
		}
	}

	// ≥¨ ±…Ë÷√Œ™0
	if (select(maxfd + 1, &r_fd_set, &w_fd_set, &e_fd_set, &wait_tv) < 0) {
		return -1;
	}

	for (int i = 0; i < idx; ++i) {
		int events = 0;
		if (FD_ISSET(ctxt[i].fd, &r_fd_set)) {
			events |= ZOOKEEPER_READ;
		}
		if (FD_ISSET(ctxt[i].fd, &w_fd_set)) {
			events |= ZOOKEEPER_WRITE;
		}
		zookeeper_process(ctxt[i].handler, events);
	}

	return 0;
}

void ZkConnMgr::SetLog(ZooLogLevel lvl) {
	zoo_set_debug_level(lvl);
}

void ZkConnMgr::void_completion_cb(int rc, const void *data) {
	ZkRequestContext* r_ctxt = (ZkRequestContext*)data;
	ZkContext* ctxt = r_ctxt->ctxt;
	ctxt->ptr->OnVoidCompletionCb(rc, r_ctxt->op.c_str(), r_ctxt->path.c_str());
	ctxt->mgr->request_alloc.dealloc(r_ctxt);
}

void ZkConnMgr::stat_completion_cb(int rc, const struct Stat *stat, const void *data) {
	ZkRequestContext* r_ctxt = (ZkRequestContext*)data;
	ZkContext* ctxt = r_ctxt->ctxt;
	ctxt->ptr->OnStatCompletionCb(rc, r_ctxt->op.c_str(), r_ctxt->path.c_str(), stat);
	ctxt->mgr->request_alloc.dealloc(r_ctxt);
}

void ZkConnMgr::string_completion_cb(int rc, const char *value, const void *data) {
	ZkRequestContext* r_ctxt = (ZkRequestContext*)data;
	ZkContext* ctxt = r_ctxt->ctxt;
	ctxt->ptr->OnStringCompletionCb(rc, r_ctxt->op.c_str(), r_ctxt->path.c_str(), value);
	ctxt->mgr->request_alloc.dealloc(r_ctxt);
}

void ZkConnMgr::data_completion_cb(int rc, const char *value, int value_len, const struct Stat *stat, const void *data) {
	ZkRequestContext* r_ctxt = (ZkRequestContext*)data;
	ZkContext* ctxt = r_ctxt->ctxt;
	ctxt->ptr->OnDataCompletionCb(rc, r_ctxt->op.c_str(), r_ctxt->path.c_str(), value, value_len, stat);
	ctxt->mgr->request_alloc.dealloc(r_ctxt);
}

void ZkConnMgr::strings_completion_cb(int rc, const struct String_vector *strings, const void *data) {
	ZkRequestContext* r_ctxt = (ZkRequestContext*)data;
	ZkContext* ctxt = r_ctxt->ctxt;
	ctxt->ptr->OnStringsCompletionCb(rc, r_ctxt->op.c_str(), r_ctxt->path.c_str(), strings);
	ctxt->mgr->request_alloc.dealloc(r_ctxt);
}

void ZkConnMgr::strings_stat_completion_cb(int rc, const struct String_vector *strings, const struct Stat *stat, const void *data) {
	ZkRequestContext* r_ctxt = (ZkRequestContext*)data;
	ZkContext* ctxt = r_ctxt->ctxt;
	ctxt->ptr->OnStringsStatCompletionCb(rc, r_ctxt->op.c_str(), r_ctxt->path.c_str(), strings, stat);
	ctxt->mgr->request_alloc.dealloc(r_ctxt);
}

void ZkConnMgr::acl_completion_cb(int rc, struct ACL_vector *acl, struct Stat *stat, const void *data) {
	ZkRequestContext* r_ctxt = (ZkRequestContext*)data;
	ZkContext* ctxt = r_ctxt->ctxt;
	ctxt->ptr->OnAclCompletionCb(rc, r_ctxt->op.c_str(), r_ctxt->path.c_str(), acl, stat);
	ctxt->mgr->request_alloc.dealloc(r_ctxt);
}
