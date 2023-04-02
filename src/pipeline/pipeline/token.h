#pragma once
#include <unordered_map>
#include <optional>
#include <variant>

namespace byfxxm {
	namespace token {
		enum class Kind {
			CON,			// 常量
			IDN,				// 标识符
			FOR,				// for
			IF,					// IF
			ELSE,			// ELSE
			//SLP,				// (
			//SRP,				// )
			LB,             // [
			RB,            // ]
			//LP,				// {
			POS,				// +
			NEG,				// -
			PLUS,			// +
			MINUS,			// -
			MUL,			// *
			DIV,				// /
			ASSIGN,				// =
			EQ,				// ==
			NE,				// !=
			LT,				// <
			LE,				// <=
			GT,				// >
			GE,				// >=
			KEOF,			// eof
			SHARP,			// #
			WHILE,			// WHILE
			ELSEIF,
			ENDIF,
			THEN,
			DO,
			END,
			NEWLINE,
			//BLANK,

			G,
			M,
			X,
			Y,
			Z,
			A,
			B,
			C,
		};

		// 种别码
		struct Token {
			Kind kind;
			std::optional<double> value;
		};

		using Mapping = std::unordered_map<std::string, token::Kind>;

		inline const Mapping keywords = {
			{"IF", Kind::IF},
			{"ELSEIF", Kind::ELSEIF},
			{"ELSE", Kind::ELSE},
			{"ENDIF", Kind::ENDIF},
			{"THEN", Kind::THEN},
			{"WHILE", Kind::WHILE},
			{"DO", Kind::DO},
			{"END", Kind::END},
		};

		inline const Mapping symbols = {
			{ "[",			Kind::LB,			},
			{ "]",			Kind::RB,			},
			//{ "{",			Kind::LP,			},
			//{ "}",			Kind::RP,			},
			//{ ";",			Kind::SEMI,		},
			{ "+",			Kind::PLUS,		},
			{ "-",			Kind::MINUS	,	},
			{ "*",			Kind::MUL	,		},
			{ "/",			Kind::DIV,			},
			{ "=",			Kind::ASSIGN,			},
			{ "==",		Kind::EQ,			},
			{ "!=",		Kind::NE,			},
			{ "<",			Kind::LT,			},
			{ "<=",		Kind::LE,			},
			{ ">",			Kind::GT,			},
			{ ">=",		Kind::GE,			},
		};

		inline const Mapping gcodes = {
			{"G", Kind::G},
			{"M", Kind::M},
			{"X", Kind::X},
			{"Y", Kind::Y},
			{"Z", Kind::Z},
		};
	}

	inline bool _IsMapping(const token::Mapping& map, const std::string& word) {
		return map.find(word) != map.end();
	}

	inline bool _IsMapping(const token::Mapping& map, char ch) {
		return std::ranges::find_if(map, [&](auto& ele) { return ch == ele.first[0]; }) != map.end();
	}

	inline bool IsKeyword(char ch) {
		return _IsMapping(token::keywords, ch);
	}

	inline bool IsKeyword(const std::string& word) {
		return _IsMapping(token::keywords, word);
	}

	inline bool IsSymbol(char ch) {
		return _IsMapping(token::symbols, ch);
	}

	inline bool IsSymbol(const std::string& word) {
		return _IsMapping(token::symbols, word);
	}

	inline bool IsGcode(char ch) {
		return _IsMapping(token::gcodes, ch);
	}

	inline bool IsGcode(const std::string& word) {
		return _IsMapping(token::gcodes, word);
	}

	inline constexpr char spaces[] = {
		' ',
		'\t',
	};

	inline bool IsSpace(char ch) {
		for (auto c : spaces) {
			if (c == ch)
				return true;
		}

		return false;
	}

	inline bool IsSharp(char ch) {
		return ch == '#';
	}

	inline constexpr char newline[] = {
		'\n',
		'\r',
	};

	inline bool IsNewline(const std::string word) {
		return word == "\n" || word == "\r\n" || word == "\r";
	}

	inline bool IsNewline(char ch) {
		return std::ranges::find(newline, ch) != std::end(newline);
	}

	inline bool IsGcode(token::Kind tok) {
		return tok == token::Kind::G
			|| tok == token::Kind::M
			|| tok == token::Kind::X
			|| tok == token::Kind::Y
			|| tok == token::Kind::Z
			|| tok == token::Kind::A
			|| tok == token::Kind::B
			|| tok == token::Kind::C
			;
	}
}
