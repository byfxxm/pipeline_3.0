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
		using Segment = std::variant<SyntaxNodeList, ClonePtr<Chunk>>;

		struct SegmentEx {
			Segment segment;
			size_t line{ 0 };
		};

		class Chunk {
		public:
			virtual ~Chunk() = default;
			virtual std::optional<SegmentEx> Next() = 0;
			virtual std::unique_ptr<Chunk> Clone() const = 0;
		};

		inline std::optional<SegmentEx> Unpack(SegmentEx&& seg) {
			if (std::holds_alternative<SyntaxNodeList>(seg.segment))
				return seg;

			auto& chunk = std::get<ClonePtr<Chunk>>(seg.segment);
			auto list = chunk->Next();
			assert(list ? std::holds_alternative<SyntaxNodeList>(list.value().segment) : true);
			return list.has_value() ? std::move(list.value()) : std::optional<SegmentEx>();
		};

		inline std::optional<SegmentEx> GetScope(std::vector<SegmentEx>&& scope, size_t& index) {
			if (index == scope.size())
				return {};

			auto& seg = scope[index];
			std::optional<SegmentEx> ret;
			if (std::holds_alternative<SyntaxNodeList>(seg.segment)) {
				ret = Unpack(std::move(seg));
				++index;
			}
			else if (std::holds_alternative<ClonePtr<Chunk>>(seg.segment)) {
				ret = Unpack(std::move(seg));
				if (!ret) {
					++index;
					ret = Unpack(std::move(scope[index++]));
				}
			}

			return ret;
		};

		class IfElse : public Chunk {
			virtual std::optional<SegmentEx> Next() override {
				if (_iscond) {
					if (_curseg > 0 && std::get<bool>(_return)) {
						--_curseg;
						_iscond = false;
						return GetScope(std::move(_segs[_curseg].scope), _scopeindex);
					}

					if (_curseg == 0)
						return Unpack(std::move(_segs[_curseg++].cond));

					if (_curseg == _segs.size()) {
						_iscond = false;
						return GetScope(std::move(_else.scope), _scopeindex);
					}

					if (!std::holds_alternative<bool>(_return))
						throw SyntaxException();

					auto cond = std::get<bool>(_return);
					if (cond) {
						_iscond = false;
						return GetScope(std::move(_segs[_curseg].scope), _scopeindex);
					}

					return Unpack(std::move(_segs[_curseg++].cond));
				}

				if (_curseg == _segs.size())
					return GetScope(std::move(_else.scope), _scopeindex);

				if (_scopeindex == _segs[_curseg].scope.size())
					return {};

				return GetScope(std::move(_segs[_curseg].scope), _scopeindex);
			}

			virtual std::unique_ptr<Chunk> Clone() const {
				return std::make_unique<IfElse>(*this);
			}

			IfElse(const Value& cond) : _return(cond) {}

			struct If {
				SegmentEx cond;
				std::vector<SegmentEx> scope;
			};

			struct Else {
				std::vector<SegmentEx> scope;
			};

			std::vector<If> _segs;
			Else _else;
			size_t _curseg{ 0 };
			bool _iscond{ true };
			const Value& _return;
			size_t _scopeindex{ 0 };
			friend class grammar::IfElse;
		};

		class While : public Chunk {
			virtual std::optional<SegmentEx> Next() override {
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
					if (!std::holds_alternative<bool>(_return))
						throw SyntaxException();

					auto cond = std::get<bool>(_return);
					if (!cond)
						return {};
				}

				_iscond = false;
				return GetScope(std::move(_scope), _scopeindex);
			}

			virtual std::unique_ptr<Chunk> Clone() const {
				return std::make_unique<While>(*this);
			}

			While(const Value& cond) : _return(cond) {}

			void _Store() {
				_scope_backup = _scope;
				_cond_backup = _cond;
			}

			void _Restore() {
				_scope = _scope_backup;
				_cond = _cond_backup;
			}

			SegmentEx _cond;
			SegmentEx _cond_backup;
			std::vector<SegmentEx> _scope;
			std::vector<SegmentEx> _scope_backup;
			bool _iscond{ true };
			const Value& _return;
			size_t _scopeindex{ 0 };
			friend class grammar::While;
		};
	}
}