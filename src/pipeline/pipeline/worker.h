#pragma once
#include <functional>

namespace byfxxm {
	struct Code;
	using WriteFunc = std::function<void(Code*)>;

	class Worker {
	public:
		virtual ~Worker() = default;
		virtual bool Do(Code*, const WriteFunc&) noexcept = 0;
	};
}
