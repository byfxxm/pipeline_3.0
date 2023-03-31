#pragma once
#include <functional>
#include <variant>
#include "token.h"
#include "predicate.h"

namespace byfxxm {
	template <class... Ts>
	inline consteval std::variant<std::remove_reference_t<Ts>...> declval(Ts&&...) noexcept {
		return {};
	}

	// 一元操作符
	using Unary = decltype(declval(
		predicate::Neg
		//, DoSharp
	));

	// 二元操作符
	using Binary = decltype(declval(
		predicate::Plus
		, predicate::Minus
		, predicate::Multi
		, predicate::Div
		, predicate::Assign
	));

	using Sharp = decltype(declval(
		predicate::Sharp
	));

	template <class... Ts>
	struct Overload : Ts...{using Ts::operator()...; };

	using Predicate = std::variant<Value, Unary, Binary, Sharp>;

	class Abstree {
	public:
		struct Node {
			Predicate pred;
			std::vector<std::unique_ptr<Node>> subs;
		};

		//Abstree(const Token&) {
		//}

        Abstree(std::unique_ptr<Node>&& root, Address& addr, bool* cond = nullptr) noexcept : _root(std::move(root)), _addr(addr), _cond(cond) {
        }

		Value Execute() {
			return _Execute(_root);
		}

	private:
		Value _Execute(std::unique_ptr<Node>& node) {
			if (std::holds_alternative<Value>(node->pred))
				return std::get<Value>(node->pred);

			std::vector<Value> v;
			std::ranges::for_each(node->subs, [&](auto&& p) {
                v.emplace_back(_Execute(p));
				});

			return std::visit(
				Overload
				{
					[&](const Value& value) {
						return value;
					},
					[&](const Unary& unary) {
						if (v.size() != 1)
							throw SyntaxException();

						return std::visit([&](auto&& func) { return func(v[0]); }, unary);
					},
					[&](const Binary& binary) {
						if (v.size() != 2)
							throw SyntaxException();

						return std::visit([&](auto&& func) { return func(v[0], v[1]); }, binary);
					},
					[&](const Sharp& sharp) {
						if (v.size() != 1)
							throw SyntaxException();

						return std::visit([&](auto&& func) { return func(v[0], _addr); }, sharp);
					}
				}, node->pred);
		}

	private:
		std::unique_ptr<Node> _root;
		Address& _addr;
		bool* _cond;
	};
}
