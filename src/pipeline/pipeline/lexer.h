#pragma once
#include <filesystem>
#include <variant>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cctype>
#include <optional>
#include <type_traits>

#include "token.h"
#include "exception.h"
#include "word.h"

namespace byfxxm {
	inline void _SkipSpace(auto&& stream) {
		while (IsSpace(stream.peek())) {
			stream.get();
		}
	}

	class Lexer {
	public:
		using StreamType = std::variant<std::ifstream, std::stringstream>;

		Lexer(const std::filesystem::path& file) : _stream(std::ifstream(file)) {
		}

		Lexer(const std::string& memory) : _stream(std::stringstream(memory)) {
		}

		void Reset(const std::filesystem::path& file) {
			using std::swap;
			auto copy = Lexer(file);
			swap(*this, copy);
		}

		void Reset(const std::string& memory) {
			using std::swap;
			auto copy = Lexer(memory);
			swap(*this, copy);
		}

		token::Token Next() {
			return std::visit(
				[this](auto&& stream)
				{
					if (stream.eof())
						return token::Token{ token::Kind::KEOF, std::nullopt };

					std::string word;
					_SkipSpace(stream);
					auto ch = stream.get();
					word.push_back(ch);

					auto peek = [&]() {return stream.peek(); };
					auto get = [&]() {return stream.get(); };
					for (const auto& p : WordsList::words) {
						if (std::optional<token::Token> tok; p->First(ch) && (tok = p->Rest(word, { peek, get, _lasttok })).has_value()) {
							_lasttok = tok;
							return tok.value();
						}
					}

					throw LexException();
				}, _stream);
		}

	private:
		StreamType _stream;
		std::optional<token::Token> _lasttok;
	};
}
