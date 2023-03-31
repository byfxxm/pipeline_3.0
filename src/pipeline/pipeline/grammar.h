#pragma once
#include "token.h"
#include "abstree.h"

namespace byfxxm {
	using SyntaxNode = std::variant<Token, std::unique_ptr<Abstree::Node>>;
	using NodeList = std::vector<SyntaxNode>;
	using Sublist = decltype(std::ranges::subrange(NodeList().begin(), NodeList().end()));

	namespace grammar {
		using Next = std::function<Token()>;

		class Grammar {
		public:
			virtual ~Grammar() = default;
			virtual bool First(const Token&) const = 0;
			virtual std::optional<NodeList> Rest(const NodeList&, Next) const = 0;
		};

		class Bracket : public Grammar {
			virtual bool First(const Token& tok) const override {
				return tok.kind == Kind::LB;
			}

			virtual std::optional<NodeList> Rest(const NodeList& list, Next next) const override {
				NodeList nodelist;
				size_t level = 1;

				while (1) {
					auto tok = next();

					if (tok.kind == Kind::LB) {
						++level;
					}
                    else if (tok.kind == Kind::RB) {
                        --level;
					}

                    if (level == 0)
                        break;

					nodelist.emplace_back(tok);
				}
				return nodelist;
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
		grammar::Bracket
	>;
}
