#pragma once
#include <unordered_map>
#include <optional>
#include <variant>

namespace byfxxm {
	inline constexpr double nan = std::numeric_limits<double>::quiet_NaN();
	inline bool IsNaN(double v) {
		return v != v;
	}

	namespace token {
		enum class Kind {
			CON,			// 常量
			IDN,				// 标识符
			FOR,				// for
			IF,					// IF
			ELSE,			// ELSE
			LB,				// [
			RB,				// ]
			POS,				// +
			NEG,				// -
			PLUS,			// +
			MINUS,			// -
			MUL,			// *
			DIV,				// /
			ASSIGN,		// =
			EQ,				// ==
			NE,				// !=
			LT,				// <
			LE,				// <=
			GT,				// >
			GE,				// >=
			KEOF,			// EOF
			SHARP,			// #
			WHILE,			// WHILE
			ELSEIF,			// ELSEIF
			ENDIF,			// ENDIF
			THEN,			// THEN
			DO,				// DO
			END,				// END
			NEWLINE,		// \r | \n | \r\n
			SEMI,			// ;
			COMMA,		// ,
			MAX,			// MAX
			MIN,				// MIN
			STRING,		// "..."
			NOT,			// NOT

			G,
			M,
			X,
			Y,
			Z,
			A,
			B,
			C,
			I,
			J,
			K,
			N,
			F,
			S,
			O,
		};

		// 种别码
		struct Token {
			Kind kind;
			std::optional<std::variant<double, std::string>> value;
		};

		using Dictionary = std::pmr::unordered_map<std::string, token::Kind>;

		inline const Dictionary keywords = {
			{"IF", Kind::IF},
			{"ELSEIF", Kind::ELSEIF},
			{"ELSE", Kind::ELSE},
			{"ENDIF", Kind::ENDIF},
			{"THEN", Kind::THEN},
			{"WHILE", Kind::WHILE},
			{"DO", Kind::DO},
			{"END", Kind::END},
			{"GT", Kind::GT},
			{"GE", Kind::GE},
			{"LT", Kind::LT},
			{"LE", Kind::LE},
			{"EQ", Kind::EQ},
			{"NE", Kind::NE},
			{"MAX", Kind::MAX},
			{"MIN", Kind::MIN},
			{"NOT", Kind::NOT},
		};

		inline const Dictionary symbols = {
			{"[", Kind::LB},
			{"]", Kind::RB},
			{"+", Kind::PLUS},
			{"-", Kind::MINUS},
			{"*", Kind::MUL},
			{"/", Kind::DIV},
			{"=", Kind::ASSIGN},
			{";", Kind::SEMI},
			{",", Kind::COMMA},
		};

		inline const Dictionary gcodes = {
			{"G", Kind::G},
			{"M", Kind::M},
			{"X", Kind::X},
			{"Y", Kind::Y},
			{"Z", Kind::Z},
			{"A", Kind::A},
			{"B", Kind::B},
			{"C", Kind::C},
			{"I", Kind::I},
			{"J", Kind::J},
			{"K", Kind::K},
			{"N", Kind::N},
			{"F", Kind::F},
			{"S", Kind::S},
			{"O", Kind::O},
		};
	}

	inline bool _IsMapping(const token::Dictionary& dict, const std::string& word) {
		return dict.contains(word);
	}

	inline bool _IsMapping(const token::Dictionary& dict, char ch) {
		return std::ranges::find_if(dict, [&](auto&& ele) {return ch == ele.first[0]; }) != dict.end();
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

	inline bool IsGcode(token::Kind kind) {
		return std::ranges::find_if(token::gcodes, [&](auto&& pair) {return pair.second == kind; }) != token::gcodes.end();
	}

	inline bool IsGcode(token::Token tok) {
		return IsGcode(tok.kind);
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

	inline void SkipSpaces(auto&& stream) {
		while (IsSpace(stream.peek())) {
			stream.get();
		}
	}

	inline bool IsSharp(char ch) {
		return ch == '#';
	}

	inline bool IsSharp(const std::string word) {
		return word == "#";
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

	template <class T>
	concept StreamConcept = requires(T t) {
		{ t.get() }->std::integral;
		{ t.peek() }->std::integral;
		{ t.eof() }->std::same_as<bool>;
	};

	template <class... Ts>
	struct Overload : Ts... {
		using Ts::operator()...;
	};

	struct Gtag {
		token::Kind code;
		double value;
	};

	using Group = std::pmr::vector<double>;
	using Value = std::variant<std::monostate, double, double*, std::string, bool, Gtag, Group>;
	using GetRetVal = std::function<Value()>;
}
