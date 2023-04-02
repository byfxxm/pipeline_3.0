#pragma once
#include <functional>

namespace byfxxm {
	namespace word {
		using Peek = std::function<char()>;
		using Get = std::function<char()>;

		class Word {
		public:
			virtual ~Word() = default;
			virtual bool First(char) const = 0;
			virtual std::optional<Token> Rest(std::string&, Peek, Get) const = 0;
		};

		class Sharp : public Word {
			virtual bool First(char ch) const override {
				return IsSharp(ch);
			}

			virtual std::optional<Token> Rest(std::string& word, Peek peek, Get get) const override {
				return Token{ Kind::SHARP, std::nullopt };
			}
		};

		class Constant : public Word {
			virtual bool First(char ch) const override {
				return isdigit(ch);
			}

			virtual std::optional<Token> Rest(std::string& word, Peek peek, Get get) const override {
				while (1) {
					auto ch = peek();
					if (!std::isdigit(ch))
						break;
					word.push_back(get());
				}

				return Token{ Kind::CON, std::stod(word) };
			}
		};

		class Key : public Word {
			virtual bool First(char ch) const override {
				return IsKeyword(ch);
			}

			virtual std::optional<Token> Rest(std::string& word, Peek peek, Get get) const override {
				while (1) {
					auto ch = peek();
					if (!std::isalpha(ch))
						break;
					word.push_back(get());
				}

				if (!IsKeyword(word))
					throw LexException();

				return Token{ keywords.at(word), std::nullopt };
			}
		};

		class Symbol : public Word {
			virtual bool First(char ch) const override {
				return IsSymbol(ch);
			}

			virtual std::optional<Token> Rest(std::string& word, Peek peek, Get get) const override {
				return Token{ symbols.at(word), std::nullopt };
			}
		};

		class Gcode : public Word {
			virtual bool First(char ch) const override {
				return IsGcode(ch);
			}

			virtual std::optional<Token> Rest(std::string& word, Peek peek, Get get) const override {
				while (1) {
					auto ch = peek();
					if (!std::isdigit(ch) && !IsSharp(ch))
						break;
					word.push_back(get());
				}

				if (!IsGcode(word))
					throw LexException();

				return Token{ symbols.at(word), std::nullopt };
			}
		};

		class Newline : public Word {
			virtual bool First(char ch) const override {
				return IsNewline(ch);
			}

			virtual std::optional<Token> Rest(std::string& word, Peek peek, Get get) const override {
				auto ch = peek();
				switch (word[0]) {
				case '\r':
					if (ch == '\n')
						word.push_back(get());
					break;

				default:
					break;
				}

				assert(IsNewline(word));
				return Token{ Kind::NEWLINE, std::nullopt };
			}
		};
	}

	template <class... _Words>
	struct _WordsList {
		static inline const std::unique_ptr<word::Word> words[] = {
			std::make_unique<_Words>()...
		};
	};

	using WordsList = _WordsList<
		word::Sharp
		, word::Constant
		, word::Key
		, word::Symbol
		, word::Gcode
		, word::Newline
	>;
}
