#pragma once
#include "worker.h"
#include "parser.h"

namespace byfxxm {
	class GParser : private Worker {
	private:
		virtual bool Do(Code*, const WriteFunc&) noexcept override {
			return true;
		}

	private:
		 std::unique_ptr<Parser> _parser;
	};
}