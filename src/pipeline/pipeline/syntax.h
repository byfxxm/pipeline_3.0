#pragma once
#include <utility>
#include <ranges>
#include <algorithm>
#include "lexer.h"
#include "production.h"
#include "address.h"
#include "grammar.h"

namespace byfxxm {

	class Syntax {
	public:
		Syntax(const std::filesystem::path& file) : _lex(file) {
		}

		Syntax(const std::string& memory) : _lex(memory) {
		}

		std::optional<Abstree> Next() {
			auto get = [&]() {
				auto tok = _lex.Get();
				if (tok.kind == token::Kind::NEWLINE)
					++_lineno;
				return tok;
			};
			auto peek = [&]() {return _lex.Peek(); };

			while (1) {
				auto tok = get();
				if (EndOfFile(tok))
					return std::nullopt;

				SyntaxNodeList list;
				list.push_back(tok);
				for (const auto& p : GrammarsList::grammars) {
					std::optional<SyntaxNodeList> sub;
					if (p->First(tok)) {
						if (!(sub = p->Rest(std::move(list), { get, peek })).has_value())
							break;
						return Abstree(expr(std::move(sub.value())), _addr);
					}
				}
			}

			throw SyntaxException();
		}

		const Address& Addr() const {
			return _addr;
		}

		const size_t Line() const {
			return _lineno;
		}

	private:
		Lexer _lex;
		size_t _lineno{ 0 };
		Address _addr;
	};
}
