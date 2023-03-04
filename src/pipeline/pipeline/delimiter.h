#pragma once
#include <string>
#include "token.h"

namespace byfxxm {
	inline const std::unordered_map<std::string, Kind> delimiter = {
		{ "(",			Kind::SLP,			},
		{ ")",			Kind::SRP,			},
		{ "{",			Kind::LP,			},
		{ "}",			Kind::RP,			},
		{ ";",			Kind::SEMI,		},
		{ "+",			Kind::PLUS,		},
		{ "-",			Kind::MINUS	,	},
		{ "*",			Kind::MUL	,		},
		{ "/",			Kind::DIV,			},
		{ "=",			Kind::ASSIGN,	},
		{ "==",		Kind::EQ,			},
		{ "!=",		Kind::NE,			},
		{ "<",			Kind::LT,			},
		{ "<=",		Kind::LE,			},
		{ ">",			Kind::GT,			},
		{ ">=",		Kind::GE,			},
	};

	class Delimiter : public Token<Delimiter> {
	public:
	};
}