﻿#pragma once
#include <functional>
#include <variant>
#include <future>
#include <cassert>
#include "token.h"
#include "predicate.h"
#include "ginterface.h"
#include "memory.h"

namespace byfxxm {
	class Abstree {
	public:
		struct Node;
		using NodePtr = ClonePtr<Node>;

		struct Node {
			Predicate pred;
			std::pmr::vector<NodePtr> subs{ &mempool };
		};

		Abstree(NodePtr&& root, Value& rval, Address& addr, Ginterface* pimpl = nullptr) noexcept : _root(std::move(root)), _return_val(rval), _addr(addr), _pimpl(pimpl) {
			assert(_root);
		}

		Value Execute() {
			_return_val = _Execute(_root);
			return _return_val;
		}

	private:
		Value _Execute(const NodePtr& node) {
			if (std::holds_alternative<Value>(node->pred))
				return std::get<Value>(node->pred);

			std::pmr::vector<Value> params{ &mempool };
			std::ranges::for_each(node->subs, [&](auto&& p) {
				params.emplace_back(_Execute(p));
				});

			return std::visit(
				Overload
				{
					[](const Value& value) {
						return value;
					},
					[&](const Unary& unary) {
						return std::visit([&](auto&& func) {return func(params[0]); }, unary);
					},
					[&](const Binary& binary) {
						return std::visit([&](auto&& func) {return func(params[0], params[1]); }, binary);
					},
					[&](const Sharp& sharp) {
						return std::visit([&](auto&& func) {return func(params[0], _addr); }, sharp);
					},
					[&](const Gcmd& gcmd) {
						return std::visit([&](auto&& func) {return func(params, _addr, _pimpl); }, gcmd);
					},
					[](const auto&)->Value { // default
						throw AbstreeException();
					},
				}, node->pred);
		}

	private:
		NodePtr _root;
		Address& _addr;
		Value& _return_val;
		Ginterface* _pimpl{ nullptr };
	};
}