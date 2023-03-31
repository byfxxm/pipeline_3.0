#pragma once
#include "lexer.h"
#include "address.h"
#include "abstree.h"
#include "syntax.h"

namespace byfxxm {
	template <class T>
	concept SyntaxConcept = requires{
		{ std::declval<T>().Next() }->std::same_as<std::optional<Abstree>>;
	};

	template <SyntaxConcept T>
	class Parser {
	public:
		Parser(T&& syn) : _syntax(std::move(syn)) {
		}

	private:
		T _syntax;
	};

	template <SyntaxConcept T>
	Parser(T&&) -> Parser<std::remove_cvref_t<T>>;
}
