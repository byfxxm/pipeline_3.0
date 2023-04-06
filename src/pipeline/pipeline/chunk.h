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
			bool cond{ false };
		};

		class Chunk {
		public:
			virtual ~Chunk() = default;
			virtual std::optional<SyntaxNodeList> Next() = 0;
		};

		class IfElse : public Chunk {
			virtual std::optional<SyntaxNodeList> Next() override {
				if (_curseg == _segs.size()) {
					if (_curseg == _else.segs.size())
						return std::nullopt;

					return std::move(_else.segs[_curseg++].list);
				}

				return std::move(_segs[_curseg++].cond.list);
			}

			IfElse(Value* p) : _cond(p) {}

			struct If {
				Segment cond;
				std::vector<Segment> segs;
			};

			struct Else {
				std::vector<Segment> segs;
			};

			friend class grammar::IfElse;
			std::vector<If> _segs;
			Else _else;
			size_t _curseg{ 0 };
			size_t _scopeindex{ 0 };
			bool _iscond{ true };
			Value* _cond{ nullptr };
		};
	}
}