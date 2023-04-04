#pragma once
#include "token.h"
#include "abstree.h"
#include "production.h"

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

		struct Utils {
			Get get;
			Peek peek;
		};

		class Grammar {
		public:
			virtual ~Grammar() = default;
			virtual bool First(const token::Token&) const = 0;
			virtual std::optional<SyntaxNodeList> Rest(SyntaxNodeList&&, const Utils&) const = 0;
		};

		class CNewseg : public Grammar {
			virtual bool First(const token::Token& tok) const override {
				return tok.kind == token::Kind::NEWLINE || tok.kind == token::Kind::SEMI;
			}

			virtual std::optional<SyntaxNodeList> Rest(SyntaxNodeList&& list, const Utils& utils) const override {
				return std::nullopt;
			}
		};

		class CExpression : public Grammar {
			virtual bool First(const token::Token& tok) const override {
				return tok.kind == token::Kind::SHARP;
			}

			virtual std::optional<SyntaxNodeList> Rest(SyntaxNodeList&& list, const Utils& utils) const override {
				while (1) {
					auto tok = utils.get();
					if (NewSegment(tok))
						break;

					list.push_back(tok);
				}

				return list;
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

			virtual std::optional<SyntaxNodeList> Rest(SyntaxNodeList&& list, const Utils& utils) const override {
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
	}

	template <class... _Gram>
	struct _GrammarsList {
		static inline const std::unique_ptr<grammar::Grammar> grammars[] = {
			std::make_unique<_Gram>()...
		};
	};

	using GrammarsList = _GrammarsList <
		grammar::CNewseg
		, grammar::CExpression
		, grammar::Ggram
	>;
}
