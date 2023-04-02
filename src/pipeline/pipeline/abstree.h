#pragma once
#include <functional>
#include <variant>
#include "token.h"
#include "predicate.h"
#include "ginterface.h"

namespace byfxxm {
	template <class... Ts>
	struct Overload : Ts...{using Ts::operator()...; };

	class Abstree {
	public:
		struct Node;
		using NodePtr = std::unique_ptr<Node>;

		struct Node {
			Predicate pred;
			std::vector<NodePtr> subs;
		};

		Abstree(NodePtr&& root, Address& addr) noexcept : _root(std::move(root)), _addr(addr) {
		}

		Value operator()(Ginterface* pimpl = nullptr) {
			return _Execute(_root, pimpl);
		}

	private:
		Value _Execute(NodePtr& node,Ginterface* pimpl) {
			if (std::holds_alternative<Value>(node->pred))
				return std::get<Value>(node->pred);

			std::vector<Value> params;
			std::ranges::for_each(node->subs, [&](auto&& p) {
				params.emplace_back(_Execute(p, pimpl));
				});

			return std::visit(
				Overload
				{
					[&](const Value& value) {
						return value;
					},
					[&](const Unary& unary) {
						return std::visit([&](auto&& func) { return func(params[0]); }, unary);
					},
					[&](const Binary& binary) {
						return std::visit([&](auto&& func) { return func(params[0], params[1]); }, binary);
					},
					[&](const Sharp& sharp) {
						return std::visit([&](auto&& func) { return func(params[0], _addr); }, sharp);
					},
					[&](const Gcmd& gcmd) {
						return std::visit([&](auto&& func) {return func(params, pimpl, _addr); }, gcmd);
					},
					[&](const auto&)->Value { // default
						throw AddressException();
					}
				}, node->pred);
		}

	private:
		NodePtr _root;
		Address& _addr;
	};
}
