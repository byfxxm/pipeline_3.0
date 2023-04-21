#pragma once
#include "abstree.h"
#include "memory.h"

namespace byfxxm {
	namespace grammar {
		class IfElse;
		class While;
	}

	namespace block {
		class Block;
	}

	struct Statement {
		std::variant<Segment, ClonePtr<block::Block>> statement{ Segment(&mempool) };
		size_t line{ 0 };
	};

	using Scope = std::pmr::vector<Statement>;

	namespace block {
		class Block {
		public:
			virtual ~Block() = default;
			virtual UniquePtr<Block> Clone() const = 0;
			virtual std::optional<Statement> Next() = 0;
		};

		inline std::optional<Statement> GetStatement(Scope& scope, size_t& index) {
			if (index == scope.size())
				return {};

			std::optional<Statement> ret;
			auto& stmt = scope[index];
			if (std::holds_alternative<Segment>(stmt.statement)) {
				ret = std::move(stmt);
				++index;
			}
			else if (std::holds_alternative<ClonePtr<Block>>(stmt.statement)) {
				auto& block = std::get<ClonePtr<Block>>(stmt.statement);
				auto next = block->Next();
				ret = next ? std::move(next.value()) : std::optional<Statement>();
				if (!ret) {
					ret = GetStatement(scope, ++index);
				}
			}

			return ret;
		};

		class IfElse : public Block {
			struct If {
				Statement cond;
				Scope scope{ &mempool };
			};

			struct Else {
				Scope scope{ &mempool };
			};

			IfElse(GetRetVal get_ret) : _get_ret(get_ret) {}

			virtual UniquePtr<Block> Clone() const {
				return MakeUnique<IfElse>(*this);
			}

			virtual std::optional<Statement> Next() override {
				if (_iscond) {
					if (_cur_stmt > 0 && std::get<bool>(_get_ret())) {
						--_cur_stmt;
						_iscond = false;
						return GetStatement(_ifs[_cur_stmt].scope, _scope_index);
					}

					if (_cur_stmt == 0)
						return std::move(_ifs[_cur_stmt++].cond);

					if (_cur_stmt == _ifs.size()) {
						_iscond = false;
						return GetStatement(_else.scope, _scope_index);
					}

					if (!std::holds_alternative<bool>(_get_ret()))
						throw SyntaxException();

					auto cond = std::get<bool>(_get_ret());
					if (cond) {
						_iscond = false;
						return GetStatement(_ifs[_cur_stmt].scope, _scope_index);
					}

					return std::move(_ifs[_cur_stmt++].cond);
				}

				if (_cur_stmt == _ifs.size())
					return GetStatement(_else.scope, _scope_index);

				if (_scope_index == _ifs[_cur_stmt].scope.size())
					return {};

				return GetStatement(_ifs[_cur_stmt].scope, _scope_index);
			}

			std::pmr::vector<If> _ifs{ &mempool };
			Else _else;
			size_t _cur_stmt{ 0 };
			bool _iscond{ true };
			GetRetVal _get_ret;
			size_t _scope_index{ 0 };
			friend class grammar::IfElse;
		};

		class While : public Block {
			While(GetRetVal get_ret) : _get_ret(get_ret) {}

			virtual UniquePtr<Block> Clone() const {
				return MakeUnique<While>(*this);
			}

			virtual std::optional<Statement> Next() override {
				if (_iscond) {
					_iscond = false;
					_Store();
					return std::move(_cond);
				}

				if (_scope_index == _scope.size()) {
					_scope_index = 0;
					_Restore();
					return std::move(_cond);
				}

				if (_scope_index == 0) {
					if (!std::holds_alternative<bool>(_get_ret()))
						throw SyntaxException();

					auto cond = std::get<bool>(_get_ret());
					if (!cond)
						return {};
				}

				_iscond = false;
				return GetStatement(_scope, _scope_index);
			}

			void _Store() {
				_scope_backup = _scope;
				_cond_backup = _cond;
			}

			void _Restore() {
				_scope = _scope_backup;
				_cond = _cond_backup;
			}

			Statement _cond;
			Statement _cond_backup;
			Scope _scope{ &mempool };
			Scope _scope_backup{ &mempool };
			bool _iscond{ true };
			GetRetVal _get_ret;
			size_t _scope_index{ 0 };
			friend class grammar::While;
		};
	}

	inline std::optional<Statement> GetStatement(ClonePtr<block::Block>& block) {
		if (block) {
			auto tree = block->Next();
			if (tree.has_value())
				return std::move(tree.value());
			else
				block.reset();
		}

		return {};
	}
}