#pragma once
#include "worker.h"
#include "gparser.h"

namespace byfxxm {
	class Gworker : private Worker {
	private:
		virtual bool Do(Code*, const WriteFunc&) noexcept override {
			return true;
		}

	private:
		std::unique_ptr<Gparser> _parser;
	};
}
