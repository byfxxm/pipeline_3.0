#pragma once
#include "lexer.h"
#include "address.h"
#include "abstree.h"
#include "syntax.h"
#include "ginterface.h"

namespace byfxxm {
	template <class T>
	concept SyntaxConcept = requires{
		{ std::declval<T>().Next() }->std::same_as<std::optional<Abstree>>;
	};

	template <SyntaxConcept T>
	class Gparser {
	public:
		Gparser(T&& syntax) : _syntax(std::move(syntax)) {
		}

		void Run(const Ginterface& pimpl) {
			while (auto abs_tree = _syntax.Next()) {
				abs_tree.value().Execute(pimpl);
			}
		}

	private:
		T _syntax;
	};

	template <SyntaxConcept T>
	Gparser(T&&) -> Gparser<std::remove_cvref_t<T>>;
}
