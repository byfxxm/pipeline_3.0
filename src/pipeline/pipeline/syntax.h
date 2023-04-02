﻿#pragma once
#include <utility>
#include <ranges>
#include <algorithm>
#include "lexer.h"
#include "abstree.h"
#include "address.h"

namespace byfxxm {
	inline constexpr size_t default_priority = -1;

	struct TokenKindTuple {
		size_t priority{ default_priority };
		Predicate pred{ Value() };
	};

	inline std::unordered_map<token::Kind, TokenKindTuple> token_kind_tuples = {
		{token::Kind::ASSIGN, {1, Binary{predicate::Assign}}},
		{token::Kind::GT, {2, Binary{predicate::GT}}},
		{token::Kind::GE, {2, Binary{predicate::GE}}},
		{token::Kind::LT, {2, Binary{predicate::LT}}},
		{token::Kind::LE, {2, Binary{predicate::LE}}},
		{token::Kind::EQ, {2, Binary{predicate::Equal}}},
		{token::Kind::NE, {2, Binary{predicate::NotEqual}}},
		{token::Kind::PLUS, {3, Binary{predicate::Plus}}},
		{token::Kind::MINUS, {3, Binary{predicate::Minus}}},
		{token::Kind::MUL, {4, Binary{predicate::Multi}}},
		{token::Kind::DIV, {4, Binary{predicate::Div}}},
		{token::Kind::SHARP, {5, Sharp{predicate::Sharp}}},
		{token::Kind::NEG, {5, Unary{predicate::Neg}}},
		{token::Kind::POS, {5, Unary{predicate::Pos}}},
		{token::Kind::CON, {}},
	};

	using SyntaxNode = std::variant<token::Token, Abstree::NodePtr>;
	using SyntaxNodeList = std::pmr::vector<SyntaxNode>;
	using SubList = decltype(std::ranges::subrange(SyntaxNodeList().begin(), SyntaxNodeList().end()));

	class Expresion {
	public:
		Abstree::NodePtr operator()(SubList sublist) const {
			return _Expression(sublist);
		}

	private:
		static Abstree::NodePtr _Expression(SubList sublist) {
			if (sublist.empty())
				return {};

			SyntaxNodeList list = _ProcessBracket(sublist, _Expression);
			auto min_pri = _FindMinPriority(list);

			auto node = _CurNode(*min_pri);
			if (auto first = _Expression(SubList(list.begin(), min_pri)))
				node->subs.emplace_back(std::move(first));
			if (auto second = _Expression(SubList(min_pri + 1, list.end())))
				node->subs.emplace_back(std::move(second));

			_CheckError(node);
			return node;
		}

		static SyntaxNodeList _ProcessBracket(SubList& list, auto&& callable) {
			SyntaxNodeList main;
			SyntaxNodeList sub;
			int level = 0;
			for (auto& node : list) {
				if (std::holds_alternative<Abstree::NodePtr>(node)) {
					main.push_back(std::move(node));
					continue;
				}

				auto tok = std::get<token::Token>(node);
				if (tok.kind == token::Kind::LB) {
					++level;
					if (level == 1)
						continue;
				}
				else if (tok.kind == token::Kind::RB) {
					--level;
					if (level == 0) {
						main.push_back(callable(sub));
						continue;
					}
				}

				if (level > 0)
					sub.push_back(tok);
				else if (level == 0)
					main.push_back(tok);
				else
					throw SyntaxException();
			}

			return main;
		}

		static SyntaxNodeList::iterator _FindMinPriority(SubList list) {
			return std::ranges::min_element(list, [](const SyntaxNode& lhs, const SyntaxNode& rhs) {
				size_t lhs_pri = default_priority;
				size_t rhs_pri = default_priority;

				if (auto p = std::get_if<token::Token>(&lhs))
					lhs_pri = token_kind_tuples.at(p->kind).priority;
				if (auto p = std::get_if<token::Token>(&rhs))
					rhs_pri = token_kind_tuples.at(p->kind).priority;

				return lhs_pri < rhs_pri;
				}
			);
		}

		static Abstree::NodePtr _CurNode(SyntaxNode& node) {
			auto ret = std::make_unique<Abstree::Node>();
			if (auto abs = std::get_if<Abstree::NodePtr>(&node)) {
				ret = std::move(*abs);
			}
			else {
				auto tok = std::get<token::Token>(node);
				ret->pred = tok.value.has_value() ? tok.value.value() : token_kind_tuples.at(tok.kind).pred;
			}

			return ret;
		}

		static void _CheckError(const Abstree::NodePtr& node) {
			std::visit(
				Overload
				{
					[&](const Value& value) {
						if (!node->subs.empty())
							throw SyntaxException();
					},
					[&](const Unary& unary) {
						if (node->subs.size() != 1)
							throw SyntaxException();
					},
					[&](const Binary& binary) {
						if (node->subs.size() != 2)
							throw SyntaxException();
					},
					[&](const Sharp& sharp) {
						if (node->subs.size() != 1)
							throw SyntaxException();
					},
					[&](const auto&) { // default
						throw SyntaxException();
					}
				}, node->pred);
		}
	};

	inline constexpr Expresion expr;

	class Syntax {
	public:
		Syntax(const std::filesystem::path& file) : _lex(file) {
		}

		Syntax(const std::string& memory) : _lex(memory) {
		}

		std::optional<Abstree> Next() {
			SyntaxNodeList list;
			while (1) {
				auto tok = _lex.Next();

				if (tok.kind == token::Kind::KEOF)
					return std::nullopt;

				if (tok.kind == token::Kind::NEWLINE) {
					++_lineno;
					if (list.empty())
						continue;
					break;
				}

				list.emplace_back(std::move(tok));
			}

			return Abstree(expr(list), _addr);
		}

		const Address& GetAddr() const {
			return _addr;
		}

	private:
		bool _Condition(Abstree::NodePtr&& absnode) {
			auto res = Abstree(std::move(absnode), _addr)();
			if (!std::holds_alternative<bool>(res))
				throw SyntaxException();

			return std::get<bool>(res);
		}

	private:
		Lexer _lex;
		size_t _lineno{ 0 };
		Address _addr;
	};
}
