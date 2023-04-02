#pragma once
#include <functional>
#include <variant>
#include "token.h"
#include "predicate.h"
#include "ginterface.h"

namespace byfxxm {
	template <class... Ts>
	inline consteval std::variant<std::remove_reference_t<Ts>...> declvariant(Ts&&...) noexcept {
		return {};
	}

	// 一元操作符
	using Unary = decltype(declvariant(
		predicate::Neg
		, predicate::Pos
	));

	// 二元操作符
	using Binary = decltype(declvariant(
		predicate::Plus
		, predicate::Minus
		, predicate::Multi
		, predicate::Div
		, predicate::Assign
	));

	using Sharp = decltype(declvariant(
		predicate::Sharp
	));

	using Gcmd = decltype(declvariant(
		predicate::Gcmd
	));

	template <class... Ts>
	struct Overload : Ts...{using Ts::operator()...; };

	// 定义谓词
	using Predicate = std::variant<Value, Unary, Binary, Sharp, Gcmd>;

	class Abstree {
	public:
		struct Node;
		using NodePtr = std::unique_ptr<Node>;

		struct Node {
			Predicate pred;
			std::vector<NodePtr> subs;
		};

		Abstree(NodePtr&& root, Address& addr, bool* cond = nullptr) noexcept : _root(std::move(root)), _addr(addr), _cond(cond) {
		}

		Value operator()(Ginterface* pimpl = nullptr) {
			return _Execute(_root, pimpl);
		}

	private:
		Value _Execute(NodePtr& node,Ginterface* pimpl) {
			if (std::holds_alternative<Value>(node->pred))
				return std::get<Value>(node->pred);

			std::vector<Value> v;
			std::ranges::for_each(node->subs, [&](auto&& p) {
				v.emplace_back(_Execute(p, pimpl));
				});

			return std::visit(
				Overload
				{
					[&](const Value& value) {
						return value;
					},
					[&](const Unary& unary) {
						return std::visit([&](auto&& func) { return func(v[0]); }, unary);
					},
					[&](const Binary& binary) {
						return std::visit([&](auto&& func) { return func(v[0], v[1]); }, binary);
					},
					[&](const Sharp& sharp) {
						return std::visit([&](auto&& func) { return func(v[0], _addr); }, sharp);
					},
					[&](const Gcmd& gcmd) {
						return std::visit([&](auto&& func) {return func(v, pimpl, _addr); }, gcmd);
					},
					[&](auto&&)->Value { // default
						throw AddressException();
					}
				}, node->pred);
		}

	private:
		NodePtr _root;
		Address& _addr;
		bool* _cond;
	};
}
