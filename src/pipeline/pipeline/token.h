#pragma once
#include <unordered_map>
#include <any>

namespace byfxxm {
	enum class Kind {
		CON,			// ����
		IDN,				// ��ʶ��
		FOR,				// for
		IF,					// if
		SLP,				// (
		SRP,				// )
		LP,				// {
		RP,				// }
		SEMI,			// ;
		PLUS,			// +
		MINUS,			// -
		MUL,			// *
		DIV,				// /
		ASSIGN,		// =
		EQ,				// ==
		NE,				// !=
		LT,				// <
		LE,				// <=
		GT,				// >
		GE,				// >=
		KEOF,			// eof
	};

	// �ֱ���
	struct TokenCode {
		Kind kind;
		std::any value;
	};

	template <class Tok>
	class Token {
	public:
		template <class Stream>
		TokenCode GetNext(const Stream& stream) {
			return static_cast<Tok*>(this)->GetNext(stream);
		}
	};
}