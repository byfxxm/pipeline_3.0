#pragma once
#include "common.h"
#include <functional>

NAMESPACE_BEGIN
struct Code;
using WriteFunc = std::function<void(Code*)>;

class Worker {
public:
	virtual ~Worker() = default;
	virtual void Process(Code*, WriteFunc) = 0;
};

NAMESPACE_END