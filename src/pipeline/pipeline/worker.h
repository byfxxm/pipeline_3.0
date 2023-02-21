#pragma once
#include <functional>

namespace byfxxm {
	struct Code;
	using WriteFunc = std::function<void(Code*)>;

	class Worker {
	public:
		virtual ~Worker() = default;
		virtual void Do(Code*, const WriteFunc&) = 0;
	};
}