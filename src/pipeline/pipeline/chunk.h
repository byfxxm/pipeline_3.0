#pragma once
#include "abstree.h"

namespace byfxxm {
	namespace grammar {
		class IfElse;
	}

	namespace chunk {
		class Chunk;
		using Segment = std::variant<SyntaxNodeList, std::unique_ptr<Chunk>>;

		struct SegmentEx {
			Segment segment;
			size_t line{ 0 };
		};

		class Chunk {
		public:
			virtual ~Chunk() = default;
			virtual std::optional<SyntaxNodeList> Next() = 0;
		};

		inline std::optional<SyntaxNodeList> SegVisitor(Segment& seg) {
			return std::visit(
				Overload{
					[&](SyntaxNodeList list)->std::optional<SyntaxNodeList> {
						return list;
					},
					[&](const std::unique_ptr<Chunk>& chunk)->std::optional<SyntaxNodeList> {
						return chunk->Next();
					},
				}, std::move(seg));
		};

		class IfElse : public Chunk {
			virtual std::optional<SyntaxNodeList> Next() override {
				auto scope = [&](std::vector<SegmentEx>& scope)->std::optional<SyntaxNodeList> {
					if (_scopeindex == scope.size())
						return {};

					auto index = _scopeindex;
					auto& seg = scope[index].segment;
					std::optional<SyntaxNodeList> ret;
					if (std::holds_alternative<SyntaxNodeList>(seg)) {
						ret = SegVisitor(seg);
						++_scopeindex;
					}
					else if (std::holds_alternative<std::unique_ptr<Chunk>>(seg)) {
						ret = SegVisitor(seg);
						if (!ret) {
							++_scopeindex;
							ret = SegVisitor(scope[_scopeindex++].segment);
						}
					}

					return ret;
				};

				if (_iscond) {
					if (_curseg > 0 && std::get<bool>(_cond)) {
						--_curseg;
						_iscond = false;
						return scope(_segs[_curseg].scope);
					}

					if (_curseg == 0)
						return SegVisitor(_segs[_curseg++].cond.segment);

					if (_curseg == _segs.size()) {
						_iscond = false;
						return scope(_else.scope);
					}

					if (!std::holds_alternative<bool>(_cond))
						throw SyntaxException();

					auto cond = std::get<bool>(_cond);
					if (cond) {
						_iscond = false;
						return scope(_segs[_curseg].scope);
					}

					return SegVisitor(_segs[_curseg++].cond.segment);
				}

				if (_curseg == _segs.size())
					return scope(_else.scope);

				if (_scopeindex == _segs[_curseg].scope.size())
					return {};

				return scope(_segs[_curseg].scope);
			}

			IfElse(const Value& cond) : _cond(cond) {}

			struct If {
				SegmentEx cond;
				std::vector<SegmentEx> scope;
			};

			struct Else {
				std::vector<SegmentEx> scope;
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