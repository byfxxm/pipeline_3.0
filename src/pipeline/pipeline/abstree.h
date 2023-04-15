#pragma once
#include <functional>
#include <variant>
#include <future>
#include <cassert>
#include "token.h"
#include "predicate.h"
#include "ginterface.h"
#include "clone_ptr.h"

namespace byfxxm {
	class Abstree {
	public:
		struct Node;
		using NodePtr = ClonePtr<Node>;

		struct Node {
			Predicate pred;
			std::vector<NodePtr> subs;
		};

		Abstree(NodePtr&& root, Address& addr, Value& retval) noexcept : _root(std::move(root)), _addr(addr), _return_val(retval) {
			assert(_root);
		}

		Value operator()(Ginterface* pimpl = nullptr) {
			_return_val = _Execute(_root, pimpl);
			return _return_val;
		}

	private:
		Value _Execute(const NodePtr& node, Ginterface* pimpl) {
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
						throw AbstreeException();
					},
				}, node->pred);
		}

	private:
		NodePtr _root;
		Address& _addr;
		Value& _return_val;
	};

	using SyntaxNode = std::variant<token::Token, Abstree::NodePtr>;
	using Segment = std::pmr::vector<SyntaxNode>;
}
