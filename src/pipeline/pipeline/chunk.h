#pragma once
#include "abstree.h"

namespace byfxxm {
	namespace grammar {
		class IfElse;
	}

	namespace chunk {
		struct Segment {
			SyntaxNodeList list;
			size_t line{ 0 };
		};

		class Chunk {
		public:
			virtual ~Chunk() = default;
			virtual std::optional<SyntaxNodeList> Next() = 0;
		};

		class IfElse : public Chunk {
			virtual std::optional<SyntaxNodeList> Next() override {
				auto scope = [&](std::vector<Segment>&& scope)->std::optional<SyntaxNodeList> {
					if (_scopeindex == scope.size())
						return std::nullopt;

					return std::move(scope[_scopeindex++].list);
				};

				if (_iscond) {
					if (_curseg == 0)
						return std::move(_segs[_curseg++].cond.list);

					if (_curseg > 0 && _curseg < _segs.size()) {
						if (!std::holds_alternative<bool>(_cond))
							throw SyntaxException();

						auto cond = std::get<bool>(_cond);
						if (cond) {
							_iscond = false;
							return scope(std::move(_segs[_curseg].scope));
						}

						return std::move(_segs[_curseg++].cond.list);
					}

					_iscond = false;
					return scope(std::move(_segs[_curseg].scope));
				}

				if (_curseg == _segs.size())
					return scope(std::move(_else.scope));

				return scope(std::move(_segs[_curseg].scope));
			}

			IfElse(const Value& cond) : _cond(cond) {}

			struct If {
				Segment cond;
				std::vector<Segment> scope;
			};

			struct Else {
				std::vector<Segment> scope;
			};

			friend class grammar::IfElse;
			std::vector<If> _segs;
			Else _else;
			size_t _curseg{ 0 };
			bool _iscond{ true };
			const Value& _cond;
			size_t _scopeindex{ 0 };
		};
	}
}