#pragma once
#include <utility>
#include <ranges>
#include <algorithm>
#include "lexer.h"
#include "production.h"
#include "address.h"
#include "grammar.h"
#include "chunk.h"

namespace byfxxm {
	class Syntax {
	public:
		Syntax(const std::filesystem::path& file) : _lex(file) {
		}

		Syntax(const std::string& memory) : _lex(memory) {
		}

		std::optional<Abstree> Next() {
			if (_remain_chunk) {
				auto tree = _remain_chunk->Next();
				if (tree.has_value())
					return Abstree(expr(std::move(tree.value())), _addr, &_return);
				else
					_remain_chunk.reset();
			}

			auto get = [&]() {
				auto tok = _lex.Get();
				if (tok.kind == token::Kind::NEWLINE)
					++_lineno;
				return tok;
			};
			auto peek = [&]() {return _lex.Peek(); };
			auto line = [&]() {return _lineno; };
			auto chunk = [&](std::unique_ptr<chunk::Chunk> chunk_) {_remain_chunk = std::move(chunk_); };
			auto retref = [&]()->Value& {return _return; };

			while (1) {
				auto tok = get();
				if (EndOfFile(tok))
					return std::nullopt;

				SyntaxNodeList list;
				list.push_back(tok);

				auto iter = std::begin(GrammarsList::grammars);
				for (; iter != std::end(GrammarsList::grammars); ++iter) {
					std::optional<SyntaxNodeList> sub;
					if ((*iter)->First(tok)) {
						if (!(sub = (*iter)->Rest(std::move(list), { get, peek, line, chunk, retref })).has_value())
							break;
						return Abstree(expr(std::move(sub.value())), _addr, &_return);
					}
				}

				if (iter == std::end(GrammarsList::grammars))
					throw SyntaxException();
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
		std::unique_ptr<chunk::Chunk> _remain_chunk;
		Value _return;
	};
}
