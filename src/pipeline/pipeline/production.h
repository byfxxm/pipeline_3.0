#pragma once
#include <vector>
#include <ranges>
#include <variant>

#include "token.h"
#include "predicate.h"
#include "abstree.h"

namespace byfxxm {
	inline constexpr size_t default_priority = -1;

	struct TokenKindTuple {
		size_t priority{ default_priority };
		Predicate pred{ Value() };
	};

	inline const std::unordered_map<token::Kind, TokenKindTuple> token_kind_tuples = {
		{token::Kind::ASSIGN, {1, Binary{predicate::Assign}}},
		{token::Kind::GT, {2, Binary{predicate::GT}}},
		{token::Kind::GE, {2, Binary{predicate::GE}}},
		{token::Kind::LT, {2, Binary{predicate::LT}}},
		{token::Kind::LE, {2, Binary{predicate::LE}}},
		{token::Kind::EQ, {2, Binary{predicate::EQ}}},
		{token::Kind::NE, {2, Binary{predicate::NE}}},
		{token::Kind::PLUS, {3, Binary{predicate::Plus}}},
		{token::Kind::MINUS, {3, Binary{predicate::Minus}}},
		{token::Kind::MUL, {4, Binary{predicate::Multi}}},
		{token::Kind::DIV, {4, Binary{predicate::Div}}},
		{token::Kind::SHARP, {5, Sharp{predicate::Sharp}}},
		{token::Kind::G, {5, Unary{predicate::Gcode<token::Kind::G>}}},
		{token::Kind::M, {5, Unary{predicate::Gcode<token::Kind::M>}}},
		{token::Kind::X, {5, Unary{predicate::Gcode<token::Kind::X>}}},
		{token::Kind::Y, {5, Unary{predicate::Gcode<token::Kind::Y>}}},
		{token::Kind::Z, {5, Unary{predicate::Gcode<token::Kind::Z>}}},
		{token::Kind::NEG, {5, Unary{predicate::Neg}}},
		{token::Kind::POS, {5, Unary{predicate::Pos}}},
		{token::Kind::CON, {}},
	};

	using SyntaxNode = std::variant<token::Token, Abstree::NodePtr>;
	using SyntaxNodeList = std::pmr::vector<SyntaxNode>;
	using SubList = decltype(std::ranges::subrange(SyntaxNodeList().begin(), SyntaxNodeList().end()));

	class Expresion {
	public:
		Abstree::NodePtr operator()(SyntaxNodeList&& list) const {
			return _Expression(list);
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
			auto less = [](const SyntaxNode& lhs, const SyntaxNode& rhs) {
				size_t lhs_pri = default_priority;
				size_t rhs_pri = default_priority;

				if (auto p = std::get_if<token::Token>(&lhs))
					lhs_pri = token_kind_tuples.at(p->kind).priority;
				if (auto p = std::get_if<token::Token>(&rhs))
					rhs_pri = token_kind_tuples.at(p->kind).priority;

				return lhs_pri < rhs_pri;
			};

			auto ret = std::ranges::min_element(list, less);
			if (auto p = std::get_if<token::Token>(&*ret);
				p && (p->kind == token::Kind::PLUS
					|| p->kind == token::Kind::MINUS
					|| p->kind == token::Kind::MUL
					|| p->kind == token::Kind::DIV))
				ret = std::ranges::min_element(list | std::views::reverse, less).base() - 1;

			return ret;
		}

		static Abstree::NodePtr _CurNode(SyntaxNode& node) {
			auto ret = std::make_unique<Abstree::Node>();
			if (auto abs = std::get_if<Abstree::NodePtr>(&node)) {
				ret = std::move(*abs);
			}
			else {
				auto tok = std::get<token::Token>(node);
				ret->pred = !IsNaN(tok.value) ? tok.value : token_kind_tuples.at(tok.kind).pred;
			}

			return ret;
		}

		static void _CheckError(const Abstree::NodePtr& node) {
			std::visit(
				Overload
				{
					[&](const Value& value) {
						if (std::holds_alternative<std::monostate>(value))
							return;

						if (!node->subs.empty())
							throw SyntaxException();
					},
					[&](const Unary&) {
						if (node->subs.size() != 1)
							throw SyntaxException();
					},
					[&](const Binary&) {
						if (node->subs.size() != 2)
							throw SyntaxException();
					},
					[&](const Sharp&) {
						if (node->subs.size() != 1)
							throw SyntaxException();
					},
					[&](const Gcmd&) {
						if (node->subs.size() == 0)
							throw SyntaxException();
					},
					[&](const auto&) { // default
						throw SyntaxException();
					}
				}, node->pred);
		}
	};

	class Gtree {
	public:
		Abstree::NodePtr operator()(SyntaxNodeList&& list) const {
			if (list.empty() || list.size() % 2 != 0)
				throw SyntaxException();

			auto root = std::make_unique<Abstree::Node>();
			root->pred = Gcmd{};
			for (auto iter = list.begin(); iter != list.end();) {
				auto node = std::make_unique<Abstree::Node>();
				node->pred = _TokToPred(std::get<token::Token>(*iter++));
				node->subs.push_back(std::move(std::get<Abstree::NodePtr>(*iter++)));
				root->subs.push_back(std::move(node));
			}

			return root;
		}

	private:
		static Predicate _TokToPred(const token::Token& tok) {
			if (!token_kind_tuples.contains(tok.kind))
				throw SyntaxException();

			return token_kind_tuples.at(tok.kind).pred;
		}
	};

	inline constexpr Expresion expr;
	inline constexpr Gtree gtree;
}
