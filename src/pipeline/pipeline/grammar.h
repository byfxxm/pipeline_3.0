#pragma once
#include "token.h"
#include "abstree.h"
#include "production.h"
#include "chunk.h"

namespace byfxxm {
	inline bool NewSegment(const token::Token& tok) {
		return tok.kind == token::Kind::NEWLINE || tok.kind == token::Kind::SEMI;
	}

	inline bool EndOfFile(const token::Token& tok) {
		return tok.kind == token::Kind::KEOF;
	}

	namespace grammar {
		using Get = std::function<token::Token()>;
		using Peek = std::function<token::Token()>;
		using Line = std::function<size_t()>;
		using Chunk = std::function<void(std::unique_ptr<chunk::Chunk>)>;
		using ReturenRef = std::function<Value&()>;

		struct Utils {
			Get get;
			Peek peek;
			Line line;
			Chunk chunk;
			ReturenRef retref;
		};

		inline SyntaxNodeList GetLine(const Utils& utils, SyntaxNodeList list = SyntaxNodeList()) {
			while (1) {
				auto tok = utils.get();
				if (NewSegment(tok))
					break;

				list.push_back(tok);
			}

			return list;
		}

		inline void SkipNewlines(const Utils& utils) {
			while (1) {
				auto tok = utils.peek();
				if (tok.kind != token::Kind::NEWLINE)
					break;

				utils.get();
			}
		}

		class Grammar {
		public:
			virtual ~Grammar() = default;
			virtual bool First(const token::Token&) const = 0;
			virtual std::optional<SyntaxNodeList> Rest(SyntaxNodeList, const Utils&) const = 0;
		};

		class Newseg : public Grammar {
			virtual bool First(const token::Token& tok) const override {
				return tok.kind == token::Kind::NEWLINE || tok.kind == token::Kind::SEMI;
			}

			virtual std::optional<SyntaxNodeList> Rest(SyntaxNodeList list, const Utils& utils) const override {
				return std::nullopt;
			}
		};

		class Expr : public Grammar {
			virtual bool First(const token::Token& tok) const override {
				return tok.kind == token::Kind::SHARP || tok.kind == token::Kind::LB;
			}

			virtual std::optional<SyntaxNodeList> Rest(SyntaxNodeList list, const Utils& utils) const override {
				return GetLine(utils, std::move(list));
			}
		};

		class Ggram : public Grammar {
			static constexpr token::Kind gcodes[] {
				token::Kind::G,
				token::Kind::M,
				token::Kind::X,
				token::Kind::Y,
				token::Kind::Z,
			};

			static bool _IsGcode(const token::Token& tok) {
				return std::ranges::find(gcodes, tok.kind) != std::end(gcodes);
			}

			virtual bool First(const token::Token& tok) const override {
				return _IsGcode(tok);
			}

			virtual std::optional<SyntaxNodeList> Rest(SyntaxNodeList list, const Utils& utils) const override {
				SyntaxNodeList gtag;
				while (1) {
					auto tok = utils.peek();
					if (NewSegment(tok)) {
						if (!gtag.empty())
							list.push_back(expr(std::move(gtag)));
						break;
					}

					if (!_IsGcode(tok)) {
						gtag.push_back(tok);
						utils.get();
						continue;
					}

					if (gtag.empty()) {
						list.push_back(tok);
						utils.get();
					}
					else {
						list.push_back(expr(std::move(gtag)));
						gtag.clear();
					}
				}

				SyntaxNodeList ret;
				ret.emplace_back(gtree(std::move(list)));
				return ret;
			}
		};

		class IfElse : public Grammar {
			virtual bool First(const token::Token& tok) const override {
				return tok.kind == token::Kind::IF;
			}

			virtual std::optional<SyntaxNodeList> Rest(SyntaxNodeList list, const Utils& utils) const override {
				auto ifelse = _NestRest(std::move(list), utils);
				auto ret = ifelse.Next();
				if (!ret.has_value())
					throw SyntaxException();

				utils.chunk(std::make_unique<chunk::IfElse>(std::move(ifelse)));
				return std::move(ret.value());
			}

			static chunk::IfElse _NestRest(SyntaxNodeList list, const Utils& utils) {
				using If = chunk::IfElse::If;
				using Else = chunk::IfElse::Else;

				auto ReadCondition = [&utils]()->chunk::SegmentEx {
					SyntaxNodeList list;
					while (1) {
						auto tok = utils.get();
						if (tok.kind == token::Kind::NEWLINE)
							throw SyntaxException();

						if (tok.kind == token::Kind::THEN)
							break;

						list.push_back(tok);
					}

					return { std::move(list), utils.line() };
				};

				auto ReadLine = [&utils]()->std::optional<chunk::SegmentEx> {
					SkipNewlines(utils);
					auto tok = utils.peek();
					if (tok.kind != token::Kind::SHARP && tok.kind != token::Kind::LB)
						return std::nullopt;

					return chunk::SegmentEx(GetLine(utils), utils.line());
				};

				auto NestIf = [&utils]()->std::optional<chunk::IfElse> {
					auto tok = utils.peek();
					if (tok.kind != token::Kind::IF)
						return {};

					SyntaxNodeList list;
					list.emplace_back(utils.get());
					return _NestRest(std::move(list), utils);
				};

				auto ReadScope = [&](std::vector<chunk::SegmentEx>& scope) {
					while (1) {
						SkipNewlines(utils);
						if (auto line = ReadLine())
							scope.push_back(std::move(line.value()));
						else if (auto nest = NestIf(); nest.has_value())
							scope.push_back(chunk::SegmentEx(chunk::Segment(std::make_unique<chunk::IfElse>(std::move(nest.value()))), utils.line()));
						else
							break;
					}
				};

				// read if
				chunk::IfElse ifelse(utils.retref());
				ifelse._segs.push_back(If(ReadCondition()));
				ReadScope(ifelse._segs.back().scope);

				// read elseif
				while (1) {
					SkipNewlines(utils);
					auto tok = utils.peek();
					if (tok.kind != token::Kind::ELSEIF)
						break;

					ifelse._segs.push_back(If(ReadCondition()));
					ReadScope(ifelse._segs.back().scope);
				}

				// read else
				auto tok = utils.get();
				if (tok.kind != token::Kind::ELSE)
					throw SyntaxException();

				SkipNewlines(utils);
				ReadScope(ifelse._else.scope);

				// endif
				tok = utils.get();
				if (tok.kind != token::Kind::ENDIF)
					throw SyntaxException();

				return ifelse;
			}
		};
	}

	template <class... _Gram>
	struct _GrammarsList {
		static inline std::unique_ptr<grammar::Grammar> grammars[]{
			std::make_unique<_Gram>()...
		};
	};

	using GrammarsList = _GrammarsList <
		grammar::Newseg
		, grammar::Expr
		, grammar::Ggram
		, grammar::IfElse
	>;
}
