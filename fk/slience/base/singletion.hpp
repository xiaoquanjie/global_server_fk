#pragma once

#include "slience/base/config.hpp"
#include "slience/base/noncopyable.hpp"

M_BASE_NAMESPACE_BEGIN

template<typename T>
class singleton : public noncopyable {
public:
	static T& mutable_instance() {
		return instance();
	}

	static const T& const_instance() {
		return instance();
	}

private:
	static T & instance() {
		static T instance;
		return instance;
	}
};


M_BASE_NAMESPACE_END
