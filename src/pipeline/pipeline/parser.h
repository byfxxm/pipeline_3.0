#pragma once
#include "lexer.h"

namespace byfxxm {
	class Parser {
	private:
		Lexer _lexer = Lexer(std::string("haha"));
	};
}