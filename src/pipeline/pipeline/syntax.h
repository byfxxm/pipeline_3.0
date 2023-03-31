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

	struct TokenKindTuple {
		size_t priority{ static_cast<size_t>(-1) };
		Predicate pred;
	};

	inline std::unordered_map<Kind, TokenKindTuple> token_kind_tuples = {
		{Kind::ASSIGN, {1, predicate::Assign}},
		{Kind::PLUS, {3, predicate::Plus}},
		{Kind::MINUS, {3, predicate::Minus}},
		{Kind::MUL, {4, predicate::Multi}},
		{Kind::DIV, {4, predicate::Div}},
		{Kind::SHARP, {5, predicate::Sharp}},
		{Kind::CON, {10}},
	};

	using SyntaxNode = std::variant<Token, std::unique_ptr<Abstree::Node>>;
	using NodeList = std::vector<SyntaxNode>;
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

			NodeList list = _SplitList(sublist, _Expression);
			auto min_priority = _FindMinPriority(list);
			auto node = _CurNode(*min_priority);

			if (auto first = _Expression(SubList(list.begin(), min_priority)))
				node->subs.emplace_back(std::move(first));
			if (auto second = _Expression(SubList(min_priority + 1, list.end())))
				node->subs.emplace_back(std::move(second));

			return node;
		}

		static NodeList _SplitList(SubList& list, auto&& callable) {
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
				if (std::holds_alternative<std::unique_ptr<Abstree::Node>>(rhs))
					return true;
				if (std::holds_alternative<std::unique_ptr<Abstree::Node>>(lhs))
					return false;

				return token_kind_tuples[std::get<Token>(lhs).kind].priority < token_kind_tuples[std::get<Token>(rhs).kind].priority;
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
