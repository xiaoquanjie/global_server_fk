#pragma once

#include "slience/base/config.hpp"

M_BASE_NAMESPACE_BEGIN

class noncopyable {
protected:
	noncopyable(const noncopyable&);
	noncopyable& operator=(const noncopyable&);
};

M_BASE_NAMESPACE_END
