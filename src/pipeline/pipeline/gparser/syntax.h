#pragma once
#include <utility>
#include <ranges>
#include <algorithm>
#include "lexer.h"
#include "production.h"
#include "address.h"
#include "grammar.h"
#include "block.h"
#include "memory.h"

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

			auto stmt = GetStatement(_remain_block);
			if (stmt) {
				_output_line = stmt.value().line;
				assert(std::holds_alternative<Segment>(stmt.value().statement));
				return _ToAbstree(std::get<Segment>(stmt.value().statement));
			}

			stmt = GetStatement(grammar::Utils{ get, peek, line, get_rval });
			if (!stmt)
				return {};

			_output_line = stmt.value().line;
			return _ToAbstree(stmt.value());
		}

		void SetGpimpl(Ginterface* pimpl) {
			_pimpl = pimpl;
		}

		const Address& Addr() const {
			return _addr;
		}

		const size_t Line() const {
			return _output_line;
		}

	private:
		Abstree _ToAbstree(Segment& seg) {
			return Abstree(expr(seg), _return_val, _addr, _pimpl);
		}

		Abstree _ToAbstree(Statement& stmt) {
			return std::visit(
				Overload{
					[this](Segment& seg)->Abstree {
						return _ToAbstree(seg);
					},
					[this](ClonePtr<block::Block>& chunk)->Abstree {
						_remain_block = std::move(chunk);
						auto stmt = GetStatement(_remain_block);
						assert(std::holds_alternative<Segment>(stmt.value().statement));
						return _ToAbstree(std::get<Segment>(stmt.value().statement));
					},
				}, stmt.statement);
		}

	private:
		Lexer<T> _lex;
		size_t _lineno{ 0 };
		size_t _output_line{ 0 };
		Value _return_val;
		Address _addr;
		Ginterface* _pimpl{ nullptr };
		ClonePtr<block::Block> _remain_block;
	};

	template <class T>
	Syntax(T) -> Syntax<T>;
}
