#pragma once
#include <utility>
#include <ranges>
#include <algorithm>
#include "lexer.h"
#include "production.h"
#include "address.h"
#include "grammar.h"
#include "chunk.h"
#include "clone_ptr.h"

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
			auto line = [&]() {return _lineno; };
			auto retval = [&]()->Value {
				if (!_return_val)
					throw SyntaxException();

				return _return_val.value();
			};

			auto stmt = GetStatement(_remain_chunk);
			if (stmt) {
				_output_line = stmt.value().line;
				assert(std::holds_alternative<Segment>(stmt.value().statement));
				return _ToAbstree(std::move(std::get<Segment>(stmt.value().statement)));
			}

			stmt = GetStatement({ get, peek, line, retval });
			if (!stmt)
				return {};

			_output_line = stmt.value().line;
			return _ToAbstree(std::move(stmt.value()));
		}

		const Address& Addr() const {
			return _addr;
		}

		const size_t Line() const {
			return _output_line;
		}

	private:
		Abstree _ToAbstree(Segment&& seg) {
			_return_val = std::nullopt;
			return Abstree(expr(std::move(seg)), _addr, _return_val);
		}

		Abstree _ToAbstree(Statement&& stmt) {
			return std::visit(
				Overload{
					[this](Segment&& seg)->Abstree {
						return _ToAbstree(std::move(seg));
					},
					[this](ClonePtr<chunk::Chunk>&& chunk)->Abstree {
						_remain_chunk = std::move(chunk);
						auto stmt = GetStatement(_remain_chunk);
						assert(std::holds_alternative<Segment>(stmt.value().statement));
						return _ToAbstree(std::get<Segment>(std::move(stmt.value().statement)));
					},
				}, std::move(stmt.statement));
		}

	private:
		Lexer _lex;
		size_t _lineno{ 0 };
		Address _addr;
		ClonePtr<chunk::Chunk> _remain_chunk;
		std::optional<Value> _return_val;
		size_t _output_line{ 0 };
	};
}
