#pragma once

#include "slience/coroutine/coroutine.h"
#include "slience/base/mutexlock.hpp"
#include "slience/base/slist.hpp"
M_COROUTINE_NAMESPACE_BEGIN

struct co_task;
struct co_task_wrapper;

// max reserve coroutine count in CoroutineTask
#ifndef M_MAX_RESERVE_COROUTINE
#define M_MAX_RESERVE_COROUTINE (20)
#endif

// max reserve task count in CoroutineTask
#ifndef M_MAX_RESERVE_TASK
#define M_MAX_RESERVE_TASK (1024)
#endif

typedef base::slist<int> intlist;

class CoroutineTask {
public:
    static void addTask(_coroutine_func_ func, void*p);

    static bool doTask();

    static void doTask(_coroutine_func_ func, void*p);

    static void clrTask();

    static void addResume(int co_id);

    static void addResume(int thrid, int co_id);

    static void resumeTask();

    static void resumeTask(int co_id);

private:
    static co_task* _get_free_task_();

    static void _recycle_free_task_(co_task* task);

    static co_task* _get_work_task_();

    static co_task_wrapper* _get_co_task_wrapper();

    static void _co_task_func_(void* p);

private:
    static std::map<int, intlist*> _thrid_idlist_map;
    static base::MutexLock _mutex;
};

M_COROUTINE_NAMESPACE_END
