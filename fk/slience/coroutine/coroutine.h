#pragma once

#include "slience/coroutine/config.hpp"
#include "slience/base/svector.hpp"
M_COROUTINE_NAMESPACE_BEGIN

struct _base_coroutine_{
	int _id;
	int _status;
	void* _data;
	_coroutine_func_ _function;
};

struct _coroutine_;

struct _base_schedule_ {
	int _cap;
	int _nco;
	unsigned int _stack_size;
	_coroutine_** _co;
	_coroutine_* _curco;
	base::svector<int> _freeid;
};

/////////////////////////////////////////////////////////

#ifdef M_PLATFORM_WIN

struct _coroutine_ : public _base_coroutine_ {
	LPVOID _ctx;
};

struct _schedule_ : public _base_schedule_ {
	_schedule_() {
		clear();
	}
	void clear() {
		_cap = 0;
		_nco = 0;
		_curco = 0;
		_ctx = 0;
		_co = 0;
		_stack_size = 0;
		_freeid.clear();
	}
	LPVOID _ctx;
};

#else

struct _coroutine_ : public _base_coroutine_ {
	char*_stack;
	int	_size;
	int	_cap;
	ucontext_t _ctx;
};

struct _schedule_ : public _base_schedule_ {
	_schedule_() {
		clear();
	}
	void clear() {
		_cap = 0;
		_nco = 0;
		_curco = 0;
		// _ctx = 0;
		_co = 0;
		_freeid.clear();
		_pri_stack = false;
		_stack_size = 0;
	}

	bool _pri_stack;
	ucontext_t _ctx;
	char _stack[M_COROUTINE_STACK_SIZE];
};

#endif

class Coroutine {
public:
	// init environment
	static bool init(unsigned int stack_size = 128 * 1204, bool pri_stack = false);

	// create one new coroutine
	static int create(_coroutine_func_ routine, void* data);

	// close
	static bool close();

	// resume
	static void resume(int co_id);

	// yield
	static void yield();

	// current coroutine id
	static unsigned int curid();

	static bool destroy(int co_id);

private:
	static void _alloc_schedule_(_schedule_& schedule);

	static _coroutine_* _alloc_co_(_schedule_& schedule, _coroutine_func_ routine, void* data);

	static bool _check_co_id(_schedule_& schedule, int co_id);

	static bool _check_resume_(_schedule_& schedule, int co_id);
};

M_COROUTINE_NAMESPACE_END
