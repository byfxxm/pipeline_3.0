#pragma once
#include "abstree.h"
#include "clone_ptr.h"

namespace byfxxm {
	namespace grammar {
		class IfElse;
		class While;
	}

	namespace chunk {
		class Chunk;
	}

	struct Statement {
		std::variant<Segment, ClonePtr<chunk::Chunk>> statement;
		size_t line{ 0 };
	};

	namespace chunk {
		class Chunk {
		public:
			virtual ~Chunk() = default;
			virtual std::unique_ptr<Chunk> Clone() const = 0;
			virtual std::optional<Statement> Next() = 0;
		};

		inline std::optional<Statement> Unpack(Statement&& stmt) {
			if (std::holds_alternative<Segment>(stmt.statement))
				return stmt;

			auto& chunk = std::get<ClonePtr<Chunk>>(stmt.statement);
			auto next = chunk->Next();
			assert(next ? std::holds_alternative<Segment>(next.value().statement) : true);
			return next ? std::move(next.value()) : std::optional<Statement>();
		};

		inline std::optional<Statement> GetScope(std::vector<Statement>& scope, size_t& index) {
			if (index == scope.size())
				return {};

			std::optional<Statement> ret;
			auto& stmt = scope[index];
			if (std::holds_alternative<Segment>(stmt.statement)) {
				ret = std::move(stmt);
				++index;
			}
			else if (std::holds_alternative<ClonePtr<Chunk>>(stmt.statement)) {
				ret = Unpack(std::move(stmt));
				if (!ret) {
					++index;
					ret = Unpack(std::move(scope[index++]));
				}
			}

			return ret;
		};

		class IfElse : public Chunk {
			struct If {
				Statement cond;
				std::vector<Statement> scope;
			};

			struct Else {
				std::vector<Statement> scope;
			};

			IfElse(GetRetVal get_ret) : _get_ret(get_ret) {}

			virtual std::unique_ptr<Chunk> Clone() const {
				return std::make_unique<IfElse>(*this);
			}

			virtual std::optional<Statement> Next() override {
				if (_iscond) {
					if (_cur_stmt > 0 && std::get<bool>(_get_ret())) {
						--_cur_stmt;
						_iscond = false;
						return GetScope(_ifs[_cur_stmt].scope, _scope_index);
					}

					if (_cur_stmt == 0)
						return Unpack(std::move(_ifs[_cur_stmt++].cond));

					if (_cur_stmt == _ifs.size()) {
						_iscond = false;
						return GetScope(_else.scope, _scope_index);
					}

					if (!std::holds_alternative<bool>(_get_ret()))
						throw SyntaxException();

					auto cond = std::get<bool>(_get_ret());
					if (cond) {
						_iscond = false;
						return GetScope(_ifs[_cur_stmt].scope, _scope_index);
					}

					return Unpack(std::move(_ifs[_cur_stmt++].cond));
				}

				if (_cur_stmt == _ifs.size())
					return GetScope(_else.scope, _scope_index);

				if (_scope_index == _ifs[_cur_stmt].scope.size())
					return {};

				return GetScope(_ifs[_cur_stmt].scope, _scope_index);
			}

			std::vector<If> _ifs;
			Else _else;
			size_t _cur_stmt{ 0 };
			bool _iscond{ true };
			GetRetVal _get_ret;
			size_t _scope_index{ 0 };
			friend class grammar::IfElse;
		};

		class While : public Chunk {
			While(GetRetVal get_ret) : _get_ret(get_ret) {}

			virtual std::unique_ptr<Chunk> Clone() const {
				return std::make_unique<While>(*this);
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
				return GetScope(_scope, _scope_index);
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
			std::vector<Statement> _scope;
			std::vector<Statement> _scope_backup;
			bool _iscond{ true };
			GetRetVal _get_ret;
			size_t _scope_index{ 0 };
			friend class grammar::While;
		};
	}

	inline std::optional<Statement> GetStatement(ClonePtr<chunk::Chunk>& chunk) {
		if (chunk) {
			auto tree = chunk->Next();
			if (tree.has_value())
				return std::move(tree.value());
			else
				chunk.reset();
		}

		return {};
	}
}