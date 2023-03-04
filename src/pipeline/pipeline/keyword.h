#pragma once
#include <string>
#include <unordered_map>
#include "token.h"

namespace byfxxm {
	inline const std::unordered_map<std::string, Kind> keywords = {
		{ "if",			Kind::IF,				},
		{ "for",		Kind::FOR,			},
	};

	class Keyword : public Token<Keyword> {
	public:
		template <class Stream>
		TokenCode GetNext(const Stream& stream) {

		}
	};
}