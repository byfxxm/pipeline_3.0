#pragma once
#include <utility>
#include <ranges>
#include <algorithm>
#include "lexer.h"
#include "abstree.h"
#include "address.h"

namespace byfxxm {
	template <class T>
	concept LexerConcept = requires{
		{ std::declval<T>().Next() }->std::same_as<Token>;
		T(std::declval<const std::filesystem::path&>());
		T(std::declval<const std::string&>());
	};

	inline constexpr size_t default_priority = -1;

	struct TokenKindTuple {
		size_t priority{ default_priority };
		Predicate pred{ Value() };
	};

	inline std::unordered_map<Kind, TokenKindTuple> token_kind_tuples = {
		{Kind::ASSIGN, {1, predicate::Assign}},
		{Kind::PLUS, {3, predicate::Plus}},
		{Kind::MINUS, {3, predicate::Minus}},
		{Kind::MUL, {4, predicate::Multi}},
		{Kind::DIV, {4, predicate::Div}},
		{Kind::SHARP, {5, predicate::Sharp}},
		{Kind::CON, {}},
	};

	using SyntaxNode = std::variant<Token, std::unique_ptr<Abstree::Node>>;
	using NodeList = std::pmr::vector<SyntaxNode>;
	using SubList = decltype(std::ranges::subrange(NodeList().begin(), NodeList().end()));

	class Expresion {
	public:
		std::unique_ptr<Abstree::Node> operator()(SubList sublist) const {
			return _Expression(sublist);
		}

	private:
		static std::unique_ptr<Abstree::Node> _Expression(SubList sublist) {
			if (sublist.empty())
				return {};

			NodeList list = _ModifyList(sublist, _Expression);
			auto min_pri = _FindMinPriority(list);
			auto node = _CurNode(*min_pri);

			if (auto first = _Expression(SubList(list.begin(), min_pri)))
				node->subs.emplace_back(std::move(first));
			if (auto second = _Expression(SubList(min_pri + 1, list.end())))
				node->subs.emplace_back(std::move(second));

			_Statement(node);
			_CheckError(node);
			return node;
		}

		static NodeList _ModifyList(SubList& list, auto&& callable) {
			NodeList main;
			NodeList sub;
			int level = 0;
			for (auto& node : list) {
				if (std::holds_alternative<std::unique_ptr<Abstree::Node>>(node)) {
					main.push_back(std::move(node));
					continue;
				}

				auto tok = std::get<Token>(node);
				if (tok.kind == Kind::LB) {
					++level;
					if (level == 1)
						continue;
				}
				else if (tok.kind == Kind::RB) {
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

		static NodeList::iterator _FindMinPriority(SubList list) {
			return std::ranges::min_element(list | std::views::reverse, [](const SyntaxNode& lhs, const SyntaxNode& rhs) {
				size_t lhs_pri = default_priority;
				size_t rhs_pri = default_priority;

				if (auto p = std::get_if<Token>(&lhs))
					lhs_pri = token_kind_tuples[p->kind].priority;
				if (auto p = std::get_if<Token>(&rhs))
					rhs_pri = token_kind_tuples[p->kind].priority;

				return lhs_pri < rhs_pri;
				}
			).base() - 1;
		}

		static std::unique_ptr<Abstree::Node> _CurNode(SyntaxNode& node) {
			auto ret = std::make_unique<Abstree::Node>();
			if (auto abs = std::get_if<std::unique_ptr<Abstree::Node>>(&node)) {
				ret = std::move(*abs);
			}
			else {
				auto tok = std::get<Token>(node);
				ret->pred = tok.value.has_value() ? tok.value.value() : token_kind_tuples.at(tok.kind).pred;
			}

			return ret;
		}

		static void _Statement(std::unique_ptr<Abstree::Node>& node) {
			if (auto binary = std::get_if<Binary>(&node->pred); binary && node->subs.size() == 1) {
				if (std::holds_alternative<decltype(predicate::Minus)>(*binary))
					node->pred = predicate::Neg;
				else if (std::holds_alternative<decltype(predicate::Plus)>(*binary))
					node->pred = predicate::Pos;
			}
		}

		static void _CheckError(const std::unique_ptr<Abstree::Node>& node) {
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
					}
				}, node->pred);
		}
	};

	inline constexpr Expresion expr;

	template <LexerConcept Lex>
	class Syntax {
	public:
		Syntax(Lex&& lex) noexcept : _lex(std::move(lex)) {
		}

		std::optional<Abstree> Next() {
			NodeList nodelist;
			while (1) {
				auto tok = _lex.Next();

				if (tok.kind == Kind::KEOF)
					return std::nullopt;

				if (tok.kind == Kind::NEWLINE) {
					++_lineno;
					if (nodelist.empty())
						continue;
					break;
				}

				nodelist.emplace_back(std::move(tok));
			}

			return Abstree(expr(nodelist), _addr);
		}

	private:
		Lex _lex;
		size_t _lineno{ 0 };
		Address _addr;
	};

	template <LexerConcept T>
	Syntax(T&&) -> Syntax<T>;
}
