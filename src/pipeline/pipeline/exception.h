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
		SyntaxException(size_t line, const char* err) : _error(std::to_string(line) + " : " + err) {
		}

	private:
		std::string _error;
	};

	class AbstreeException : public ParseException {
	public:
		using ParseException::ParseException;
	};
}