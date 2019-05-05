#pragma once

#define M_COROUTINE_NAMESPACE_BEGIN namespace coroutine{
#define M_COROUTINE_NAMESPACE_END }

#include "slience/base/config.hpp"

#ifdef M_PLATFORM_WIN
#include "slience/base/win.hpp"
#else
#include <pthread.h>
#include <ucontext.h>
#endif

// common header
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#ifndef COROUTINE_READY
#define COROUTINE_READY   (1)
#endif

#ifndef COROUTINE_RUNNING
#define COROUTINE_RUNNING (2)
#endif

#ifndef COROUTINE_SUSPEND
#define COROUTINE_SUSPEND (3)
#endif

#ifndef COROUTINE_DEAD
#define COROUTINE_DEAD	  (4)
#endif

#ifndef DEFAULT_COROUTINE
#define DEFAULT_COROUTINE (1024)
#endif

// invalid coroutine id
#ifndef M_INVALID_COROUTINE_ID
#define M_INVALID_COROUTINE_ID (-1)
#endif

// main coroutine id
#ifndef M_MAIN_COROUTINE_ID
#define M_MAIN_COROUTINE_ID (0)
#endif

// for private stack
#ifndef M_COROUTINE_STACK_SIZE
#define M_COROUTINE_STACK_SIZE  4*1024*1024
#endif

// coroutine func type
typedef void(*_coroutine_func_)(void*ud);

