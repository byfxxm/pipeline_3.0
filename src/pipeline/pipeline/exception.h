#pragma once
#include <exception>

namespace byfxxm{
	class ParseException : public std::exception {
	public:
		ParseException() = default;
		using std::exception::exception;
	};

	class LexException : public ParseException {
	public:
		using ParseException::ParseException;
	};

	class SyntaxException : public ParseException {
	public:
		using ParseException::ParseException;
	};

	class AddressException : public ParseException {
	public:
		using ParseException::ParseException;
	};
}