#pragma once
#include <exception>

namespace byfxxm{
	class ParseException : public std::exception {

	};

	class LexException : public ParseException {

	};

	class SyntaxException : public ParseException {

	};

	class AddressException : public ParseException {

	};
}