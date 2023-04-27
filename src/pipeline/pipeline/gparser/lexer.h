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
	inline void SkipSpaces(auto&& stream) {
		while (IsSpace(stream.peek())) {
			stream.get();
		}
	}

	template <StreamConcept T>
	class Lexer {
	public:
		Lexer(T&& stream) : _stream(std::move(stream)) {}

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

		token::Token Get() {
			_lasttok = Peek();
			_peektok.reset();
			return _lasttok.value();
		}

		token::Token Peek() {
			if (!_peektok.has_value())
				_peektok = _Next();

			return _peektok.value();
		}

	private:
		token::Token _Next() {
			if (_stream.eof())
				return token::Token{ token::Kind::KEOF, nan };

			SkipSpaces(_stream);
			auto ch = _stream.get();
			std::string word;
			word.push_back(ch);

			auto peek = [&]() {return _stream.peek(); };
			auto get = [&]() {return _stream.get(); };
			auto last = [&]()->const std::optional<token::Token>&{return _lasttok; };
			for (const auto& p : word::WordsList::words) {
				std::optional<token::Token> tok;
				if (p->First(ch) && (tok = p->Rest(word, { peek, get, last })).has_value()) {
					return tok.value();
				}
			}

			throw LexException();
		}

	private:
		T _stream;
		std::optional<token::Token> _lasttok;
		std::optional<token::Token> _peektok;
	};

	template <class T>
	Lexer(T) -> Lexer<T>;
}
