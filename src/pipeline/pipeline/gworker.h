#pragma once
#include "worker.h"
#include "gparser/gparser.h"

namespace byfxxm {
	class Gworker : private Worker {
	private:
		virtual bool Do(Code*, const WriteFunc&) noexcept override {
			return true;
		}

	private:
		std::variant<std::monostate, Gparser<std::fstream>, Gparser<std::stringstream>> _parser;
	};
}
