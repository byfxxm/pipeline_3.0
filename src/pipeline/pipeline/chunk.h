#pragma once
#include "abstree.h"
#include "clone_ptr.h"
#include "grammar.h"

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
			virtual std::optional<Statement> Next() = 0;
			virtual std::unique_ptr<Chunk> Clone() const = 0;
		};

		inline std::optional<Statement> Unpack(Statement&& stmt) {
			if (std::holds_alternative<Segment>(stmt.statement))
				return stmt;

			auto& chunk = std::get<ClonePtr<Chunk>>(stmt.statement);
			auto list = chunk->Next();
			assert(list ? std::holds_alternative<Segment>(list.value().statement) : true);
			return list ? std::move(list.value()) : std::optional<Statement>();
		};

		inline std::optional<Statement> GetScope(std::vector<Statement>&& scope, size_t& index) {
			if (index == scope.size())
				return {};

			auto& stmt = scope[index];
			std::optional<Statement> ret;
			if (std::holds_alternative<Segment>(stmt.statement)) {
				ret = Unpack(std::move(stmt));
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
			virtual std::optional<Statement> Next() override {
				if (_iscond) {
					if (_cur_stmt > 0 && std::get<bool>(_return())) {
						--_cur_stmt;
						_iscond = false;
						return GetScope(std::move(_ifs[_cur_stmt].scope), _scopeindex);
					}

					if (_cur_stmt == 0)
						return Unpack(std::move(_ifs[_cur_stmt++].cond));

					if (_cur_stmt == _ifs.size()) {
						_iscond = false;
						return GetScope(std::move(_else.scope), _scopeindex);
					}

					if (!std::holds_alternative<bool>(_return()))
						throw SyntaxException();

					auto cond = std::get<bool>(_return());
					if (cond) {
						_iscond = false;
						return GetScope(std::move(_ifs[_cur_stmt].scope), _scopeindex);
					}

					return Unpack(std::move(_ifs[_cur_stmt++].cond));
				}

				if (_cur_stmt == _ifs.size())
					return GetScope(std::move(_else.scope), _scopeindex);

				if (_scopeindex == _ifs[_cur_stmt].scope.size())
					return {};

				return GetScope(std::move(_ifs[_cur_stmt].scope), _scopeindex);
			}

			virtual std::unique_ptr<Chunk> Clone() const {
				return std::make_unique<IfElse>(*this);
			}

			IfElse(RValue retval) : _return(retval) {}

			struct If {
				Statement cond;
				std::vector<Statement> scope;
			};

			struct Else {
				std::vector<Statement> scope;
			};

			std::vector<If> _ifs;
			Else _else;
			size_t _cur_stmt{ 0 };
			bool _iscond{ true };
			RValue _return;
			size_t _scopeindex{ 0 };
			friend class grammar::IfElse;
		};

		class While : public Chunk {
			virtual std::optional<Statement> Next() override {
				if (_iscond) {
					_iscond = false;
					_Store();
					return std::move(_cond);
				}

				if (_scopeindex == _scope.size()) {
					_scopeindex = 0;
					_Restore();
					return std::move(_cond);
				}

				if (_scopeindex == 0) {
					if (!std::holds_alternative<bool>(_return()))
						throw SyntaxException();

					auto cond = std::get<bool>(_return());
					if (!cond)
						return {};
				}

				_iscond = false;
				return GetScope(std::move(_scope), _scopeindex);
			}

			virtual std::unique_ptr<Chunk> Clone() const {
				return std::make_unique<While>(*this);
			}

			While(RValue retval) : _return(retval) {}

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
			RValue _return;
			size_t _scopeindex{ 0 };
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