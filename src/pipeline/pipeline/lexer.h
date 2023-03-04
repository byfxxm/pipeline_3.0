#pragma once
#include <filesystem>
#include <variant>
#include <fstream>
#include <sstream>
#include <cassert>
#include "token.h"
#include "keyword.h"
#include "space.h"
#include "delimiter.h"

namespace byfxxm {
	class Lexer {
	public:
		Lexer(const std::filesystem::path& file) : _stream(std::ifstream(file)) {
		}

		Lexer(const std::string& memory) : _stream(std::stringstream(memory)) {
		}

		void Reset(const std::filesystem::path& file) {
			_stream = std::ifstream(file);
		}

		void Reset(const std::string& memory) {
			_stream = std::stringstream(memory);
		}

		TokenCode GetNextToken() {
			if (std::get_if<std::ifstream>(&_stream))
				return _GetNextToken<std::ifstream>();

			if (std::get_if<std::stringstream>(&_stream))
				return _GetNextToken<std::stringstream>();

			assert(0);
			return {};
		}

	private:
		template <class Stream>
		TokenCode _GetNextToken() {
			auto& stream = std::get<Stream>(_stream);
			std::string word;
			while (!_SkipSpace<Stream>()) {
				auto ch = stream.peek();
				word.push_back(stream.get());
				//if (IsKeyword(word)) {
				//	break;
				//}
			}

			return { Kind::KEOF, nullptr};
		}

		template <class Stream>
		bool _SkipSpace() {
			auto& stream = std::get<Stream>(_stream);
			while (!stream.eof()) {
				if (!Space::IsSpace(stream.peek()))
					return stream.eof();

				stream.get();
			}

			return true;
		}


	private:
		std::variant<std::ifstream, std::stringstream> _stream;
	};
}