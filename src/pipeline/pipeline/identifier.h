#pragma once
#include "token.h"

namespace byfxxm {
	class Identifier : public Token<Identifier> {
	public:
		static constexpr char Alphabet[] = {
			'a', 'b'
		};
	};
}