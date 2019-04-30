#include "slience/coroutine/courtouine_task.h"
#include "slience/base/tls.hpp"
#include "slience/base/svector.hpp"
#include <set>
#include "slience/base/thread.hpp"

M_COROUTINE_NAMESPACE_BEGIN

struct co_task {
    void* p;
    _coroutine_func_ func;
};

struct co_task_wrapper {
   int co_id;
   co_task* task;
};

typedef base::slist<co_task*> tasklist;
typedef base::svector<co_task*> taskvector;
typedef base::slist<co_task_wrapper*> taskwrapperlist;
typedef base::svector<co_task_wrapper*> taskwrappervector;
typedef std::set<co_task_wrapper*> taskwrapperset;

#define gfreetaskvec base::tlsdata<taskvector,0>::data()
#define gworktasklist base::tlsdata<tasklist,0>::data()
#define gfreecovec	base::tlsdata<taskwrappervector,0>::data()
#define galltaskwrapperset base::tlsdata<taskwrapperset,0>::data()

std::map<int, intlist*> CoroutineTask::_thrid_idlist_map;
base::MutexLock CoroutineTask::_mutex;

void CoroutineTask::addTask(_coroutine_func_ func, void*p) {
    co_task* task = _get_free_task_();
    task->func = func;
    task->p = p;
    gworktasklist.push_back(task);
}

bool CoroutineTask::doTask() {
    if (Coroutine::curid() == M_MAIN_COROUTINE_ID) {
        co_task* task = _get_work_task_();
        if (task) {
            co_task_wrapper* wrapper = _get_co_task_wrapper();
            wrapper->task = task;
            Coroutine::resume(wrapper->co_id);
            return true;
        }
    }
	return false;
}

void CoroutineTask::doTask(_coroutine_func_ func, void*p) {
    if (Coroutine::curid() == M_MAIN_COROUTINE_ID) {
        co_task* task = _get_free_task_();
        task->func = func;
        task->p = p;
        co_task_wrapper* wrapper = _get_co_task_wrapper();
        wrapper->task = task;
        Coroutine::resume(wrapper->co_id);
    }
}

void CoroutineTask::clrTask() {
    if (Coroutine::curid() != M_MAIN_COROUTINE_ID) {
        return;
    }
    // kill all coroutine
    taskwrapperset& all_task_wrapper_set = galltaskwrapperset;
    for (auto iter = all_task_wrapper_set.begin(); iter != all_task_wrapper_set.end();
            ++iter) {
        Coroutine::destroy((*iter)->co_id);
    }
    all_task_wrapper_set.clear();

    // free all task
    taskvector& tl = gfreetaskvec;
    while (tl.size()) {
        free(tl.back());
        tl.pop_back();
    }
    tasklist& wtl = gworktasklist;
    while (wtl.size()) {
        free(wtl.front());
        wtl.pop_front();
    }

    // clear resum list
    unsigned int thrid = base::thread::ctid();
    base::ScopedLock scoped(_mutex);
    std::map<int, intlist*>::iterator iter = _thrid_idlist_map.find(thrid);
    if (iter != _thrid_idlist_map.end()) {
        iter->second->clear();
    }
}

void CoroutineTask::addResume(int co_id) {
    unsigned int thrid = base::thread::ctid();
    addResume(thrid, co_id);
}

void CoroutineTask::addResume(int thrid, int co_id) {
    _mutex.lock();
    std::map<int, intlist*>::iterator iter = _thrid_idlist_map.find(thrid);
    if (iter != _thrid_idlist_map.end()) {
        iter->second->push_back(co_id);
    }
    else {
        intlist* pslist = new intlist;
        pslist->push_back(co_id);
        _thrid_idlist_map[thrid] = pslist;
    }
    _mutex.unlock();
}

void CoroutineTask::resumeTask() {
    if (Coroutine::curid() != M_MAIN_COROUTINE_ID) {
        return;
    }
    unsigned int thrid = base::thread::ctid();
    intlist idlist;
    _mutex.lock();
    std::map<int, intlist*>::iterator iter = _thrid_idlist_map.find(thrid);
    if (iter != _thrid_idlist_map.end()) {
        iter->second->swap(idlist);
    }
    _mutex.unlock();

    while (idlist.size()) {
        int id = idlist.front();
        idlist.pop_front();
        Coroutine::resume(id);
    }
}

void CoroutineTask::resumeTask(int co_id) {
    if (Coroutine::curid() == M_MAIN_COROUTINE_ID) {
        Coroutine::resume(co_id);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

co_task* CoroutineTask::_get_free_task_() {
    taskvector& tl = gfreetaskvec;
    co_task* task = 0;
    if (tl.empty()) {
        task = (co_task*)malloc(sizeof(co_task));
    } else {
        task = tl.back();
        tl.pop_back();
    }
    return task;
}

void CoroutineTask::_recycle_free_task_(co_task* task) {
    taskvector& tl = gfreetaskvec;
    if (tl.size() >= M_MAX_RESERVE_TASK) {
        free(task);
    } else {
        tl.push_back(task);
    }
}

co_task* CoroutineTask::_get_work_task_() {
    tasklist& tl = gworktasklist;
    if (tl.empty()) {
        return 0;
    }
    else {
        co_task* task = tl.front();
        tl.pop_front();
        return task;
    }
}

co_task_wrapper* CoroutineTask::_get_co_task_wrapper() {
    co_task_wrapper* wrapper = 0;
    taskwrappervector& tw = gfreecovec;
    if (!tw.empty()) {
        wrapper = tw.back();
        tw.pop_back();
    }
    else {
        wrapper = (co_task_wrapper*)malloc(sizeof(co_task_wrapper));
        wrapper->co_id = Coroutine::create(_co_task_func_, wrapper);
        galltaskwrapperset.insert(wrapper);
    }
    wrapper->task = 0;
    return wrapper;
}

void CoroutineTask::_co_task_func_(void* p) {
    co_task_wrapper* wrapper = (co_task_wrapper*)p;
    taskwrappervector& tw = gfreecovec;
    co_task* task = 0;
    while (true) {
        task = wrapper->task;
        if (!task) {
            break;
        }
        task->func(task->p);
        _recycle_free_task_(task);
        if (galltaskwrapperset.size() >= M_MAX_RESERVE_COROUTINE) {
            break;
        }
        wrapper->task = 0;
        tw.push_back(wrapper);
        Coroutine::yield();
    }
    galltaskwrapperset.erase(wrapper);
    free(wrapper);
}

M_COROUTINE_NAMESPACE_END