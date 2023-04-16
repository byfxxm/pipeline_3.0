﻿#pragma once
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
	template <StreamConcept T>
	class Syntax {
	public:
		Syntax(T&& stream) : _lex(std::move(stream)) {}

		std::optional<Abstree> Next() {
			auto get = [&]() {
				auto tok = _lex.Get();
				if (tok.kind == token::Kind::NEWLINE)
					++_lineno;
				return tok;
			};
			auto peek = [&]() {return _lex.Peek(); };
			auto line = [&]() {return _lineno; };
			auto get_rval = [&]()->Value {return _return_val; };

			auto stmt = GetStatement(_remain_chunk);
			if (stmt) {
				_output_line = stmt.value().line;
				assert(std::holds_alternative<ProgSeg>(stmt.value().statement));
				return _ToAbstree(std::move(std::get<ProgSeg>(stmt.value().statement)));
			}

			stmt = GetStatement(grammar::Utils{ get, peek, line, get_rval });
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
		Abstree _ToAbstree(ProgSeg&& seg) {
			return Abstree(expr(std::move(seg)), _addr, _return_val);
		}

		Abstree _ToAbstree(Statement&& stmt) {
			return std::visit(
				Overload{
					[this](ProgSeg&& seg)->Abstree {
						return _ToAbstree(std::move(seg));
					},
					[this](ClonePtr<chunk::Chunk>&& chunk)->Abstree {
						_remain_chunk = std::move(chunk);
						auto stmt = GetStatement(_remain_chunk);
						assert(std::holds_alternative<ProgSeg>(stmt.value().statement));
						return _ToAbstree(std::get<ProgSeg>(std::move(stmt.value().statement)));
					},
				}, std::move(stmt.statement));
		}

	private:
		Lexer<T> _lex;
		size_t _lineno{ 0 };
		Address _addr;
		ClonePtr<chunk::Chunk> _remain_chunk;
		size_t _output_line{ 0 };
		Value _return_val;
	};

	template <class T>
	Syntax(T) -> Syntax<T>;
}
