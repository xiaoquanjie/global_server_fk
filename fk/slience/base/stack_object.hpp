/*
 * 设计的初衷是可以对象复用，提高程序的内存效率，甚至是运行效率，但是同时又不需要像直接
 * 使用对象池那样需要显示的回收对象。stackobject被设计于只能用于栈中，这是避免出现stackobject
 * 对象本身造成内存碎片的情况。需要在正确的场景下使用。
 * stackobject对象符合以下原则
 * 1、支持对托管对象的内存复用
 * 2、托管对象的内存自动回收
 * 3、调用自然
 * 4、支持参数传递
 * 5、不支持new
 * 6、不支持赋值调用
*/
#pragma once

#include "slience/base/object_pool.h"

M_BASE_NAMESPACE_BEGIN

/*
 * 1、目前只支持最多五个构造函数
 * 2、Allocator默认是ObjectPool<T, 1000, base::MutexLock>
*/
template<typename T, typename Allocator = ObjectPool<T, 1000, base::MutexLock>>
class StackObject {
public:
	~StackObject() {
		*_refs -= 1;
		if (*_refs == 0) {
			delete _refs;
			Allocator::Dealloc(_obj);
		}
	}

	T* operator ->() {
		return _obj;
	}

	const T* operator ->() const {
		return _obj;
	}

	StackObject(const StackObject& other) {
		_refs = other._refs;
		*_refs += 1;
		_obj = other._obj;
	}

	StackObject() {
		_obj = Allocator::Alloc();
		initref();
	}

	template<class P>
	StackObject(const P& p) {
		_obj = Allocator::Alloc(p);
		initref();
	}

	template<class P1, class P2>
	StackObject(const P1& p1, const P2& p2) {
		_obj = Allocator::Alloc(p1, p2);
		initref();
	}
	
	template<class P1, class P2, class P3>
	StackObject(const P1& p1, const P2& p2, const P3& p3) {
		_obj = Allocator::Alloc(p1, p2, p3);
		initref();
	}

	template<class P1, class P2, class P3, class P4>
	StackObject(const P1& p1, const P2& p2, const P3& p3, const P4& p4) {
		_obj = Allocator::Alloc(p1, p2, p3, p4);
		initref();
	}

	template<class P1, class P2, class P3, class P4, class P5>
	StackObject(const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5) {
		_obj = Allocator::Alloc(p1, p2, p3, p4, p5);
		initref();
	}

protected:
	void initref() {
		_refs = new int;
		*_refs = 1;
	}

protected:
	// 屏蔽new
	void* operator new(size_t size);
	void* operator new[](size_t size);
	// 不支持赋值,避免被滥用影响性能
	StackObject& operator = (const StackObject& other);
protected:
	T* _obj;
	int* _refs;
};

M_BASE_NAMESPACE_END
