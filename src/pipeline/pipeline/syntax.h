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
	inline std::optional<chunk::SegmentEx> GetSegment(const Utils& utils) {
		while (1) {
			auto tok = utils.peek();
			if (EndOfFile(tok))
				return {};

			SyntaxNodeList list;
			list.push_back(utils.get());

			auto iter = std::begin(GrammarsList::grammars);
			for (; iter != std::end(GrammarsList::grammars); ++iter) {
				std::optional<chunk::SegmentEx> sub;
				if ((*iter)->First(tok)) {
					if (!(sub = (*iter)->Rest(std::move(list), utils)).has_value())
						break;
					return std::move(sub.value());
				}
			}

			if (iter == std::end(GrammarsList::grammars))
				throw SyntaxException();
		}

		throw SyntaxException();
	}

	inline std::optional<chunk::SegmentEx> GetFromChunk(ClonePtr<chunk::Chunk>& chunk) {
		if (chunk) {
			auto tree = chunk->Next();
			if (tree.has_value())
				return std::move(tree.value());
			else
				chunk.reset();
		}

		return {};
	}

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
			auto retref = [&]()->Value& {return _return; };

			auto seg = GetFromChunk(_remain_chunk);
			if (seg) {
				_output_line = seg.value().line;
				assert(std::holds_alternative<SyntaxNodeList>(seg.value().segment));
				return _ToAbstree(std::move(std::get<SyntaxNodeList>(seg.value().segment)));
			}

			seg = GetSegment({ get, peek, line, retref });
			if (!seg)
				return {};

			_output_line = seg.value().line;
			return std::visit(
				Overload{
					[this](SyntaxNodeList&& list)->std::optional<Abstree> {
						return _ToAbstree(std::move(list));
					},
					[this](ClonePtr<chunk::Chunk>&& chunk)->std::optional<Abstree> {
						_remain_chunk = std::move(chunk);
						auto seg = GetFromChunk(_remain_chunk);
						assert(std::holds_alternative<SyntaxNodeList>(seg.value().segment));
						return _ToAbstree(std::get<SyntaxNodeList>(std::move(seg.value().segment)));
					},
				}, std::move(seg.value().segment)
			);
		}

		const Address& Addr() const {
			return _addr;
		}

		const size_t Line() const {
			return _output_line;
		}

	private:
		Abstree _ToAbstree(SyntaxNodeList&& list) {
			return Abstree(expr(std::move(list)), _addr, &_return);
		}

	private:
		Lexer _lex;
		size_t _lineno{ 0 };
		Address _addr;
		ClonePtr<chunk::Chunk> _remain_chunk;
		Value _return;
		size_t _output_line{ 0 };
	};
}
