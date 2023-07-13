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
	inline std::optional<Statement> GetStatement(ClonePtr<block::Block>& block) {
		if (block) {
			auto tree = block->Next();
			if (tree.has_value())
				return std::move(tree.value());
			else
				block.Reset();
		}

		return {};
	}

	using AbstreeWithLineno = std::tuple<Abstree, size_t>;

	template <StreamConcept T>
	class Syntax {
	public:
		Syntax(T&& stream) : _lex(std::move(stream)) {}

		std::optional<AbstreeWithLineno> Next() {
			auto stmt = GetStatement(_remain_block);
			if (stmt) {
				_output_lineno = stmt.value().line;
				assert(std::holds_alternative<Segment>(stmt.value().statement));
				return AbstreeWithLineno{ _ToAbstree(std::get<Segment>(std::move(stmt.value().statement))), _output_lineno };
			}

			auto get = [this]() {
				auto tok = _lex.Get();
				if (tok.kind == token::Kind::NEWLINE || tok.kind == token::Kind::KEOF)
					++_lineno;
				return tok;
			};
			auto peek = [this]() {return _lex.Peek(); };
			auto line = [this]() {return _lineno; };
			auto get_rval = [this]()->Value {return _return_val; };
			stmt = GetStatement(grammar::Utils{ get, peek, line, get_rval });
			if (!stmt)
				return {};

			_output_lineno = stmt.value().line;
			return AbstreeWithLineno{ _ToAbstree(std::move(stmt.value())), _output_lineno };
		}

		void Set(Address* addr, Ginterface* pimpl) {
			_addr = addr;
			_pimpl = pimpl;
		}

	private:
		Abstree _ToAbstree(Segment&& seg) {
			return Abstree(expr(seg), _return_val, _addr, _pimpl);
		}

		Abstree _ToAbstree(Statement&& stmt) {
			return std::visit(
				Overload{
					[this](Segment&& seg)->Abstree {
						return _ToAbstree(std::move(seg));
					},
					[this](ClonePtr<block::Block>&& chunk)->Abstree {
						_remain_block = std::move(chunk);
						auto stmt = GetStatement(_remain_block);
						assert(std::holds_alternative<Segment>(stmt.value().statement));
						_output_lineno = stmt.value().line;
						return _ToAbstree(std::get<Segment>(std::move(stmt.value().statement)));
					},
				}, std::move(stmt.statement));
		}

	private:
		Lexer<T> _lex;
		size_t _lineno{ 1 };
		size_t _output_lineno{ 1 };
		Value _return_val;
		Address* _addr{ nullptr };
		Ginterface* _pimpl{ nullptr };
		ClonePtr<block::Block> _remain_block;
	};

	template <class T>
	Syntax(T) -> Syntax<T>;
}
