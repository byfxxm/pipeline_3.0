#pragma once
#include "token.h"
#include "address.h"

#define IsSameType(lhs, rhs) (std::is_same_v<decltype(lhs), decltype(rhs)>)
#define IsType(v, type) (std::is_same_v<std::remove_cvref_t<decltype(v)>, type>)
#define IsDouble(v) IsType(v, double)
#define IsString(v) IsType(v, std::string)
#define IsDoublePtr(v) IsType(v, double*)

namespace byfxxm {
	namespace predicate {
		inline auto Plus = [](const Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					return Value{ *l + *r };
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					return Value{ *l + r };
				else if constexpr (IsDouble(l) && IsDoublePtr(r))
					return Value{ l + *r };
				else if constexpr (IsDouble(l) && IsDouble(r))
					return Value{ l + r };
				else if constexpr (IsString(l) && IsString(r))
					return Value{ l + r };
				else
					throw SyntaxException();
				}, lhs, rhs);
		};

		inline auto Minus = [](const Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					return Value{ *l - *r };
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					return Value{ *l - r };
				else if constexpr (IsDouble(l) && IsDoublePtr(r))
					return Value{ l - *r };
				else if constexpr (IsDouble(l) && IsDouble(r))
					return Value{ l - r };
				else
					throw SyntaxException();
				}, lhs, rhs);
		};

		inline auto Multi = [](const Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					return Value{ *l * *r };
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					return Value{ *l * r };
				else if constexpr (IsDouble(l) && IsDoublePtr(r))
					return Value{ l * *r };
				else if constexpr (IsDouble(l) && IsDouble(r))
					return Value{ l * r };
				else
					throw SyntaxException();
				}, lhs, rhs);
		};

		inline auto Div = [](const Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					return Value{ *l / *r };
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					return Value{ *l / r };
				else if constexpr (IsDouble(l) && IsDoublePtr(r))
					return Value{ l / *r };
				else if constexpr (IsDouble(l) && IsDouble(r))
					return Value{ l / r };
				else
					throw SyntaxException();
				}, lhs, rhs);
		};

		inline auto Assign = [](Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					*l = *r;
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					*l = r;
				else
					throw SyntaxException();

				return Value{ l };
				}, lhs, rhs);

			//if (lhs.index() != rhs.index())
			//	throw SyntaxException();

			//lhs = rhs;
			//return lhs;
		};

		inline auto Neg = [](const Value& value) {
			return std::visit([](auto&& v)->Value {
				if constexpr (IsDouble(v))
					return Value{ -v };
				else
					throw SyntaxException();
				}, value);
		};

		inline auto Sharp = [](const Value& value, Address& addr) {
			return std::visit([&](auto&& v)->Value {
                if constexpr (IsDouble(v))
                    return addr[v].get();
                else if constexpr (IsDoublePtr(v))
                    return addr[*v].get();
				else
					throw SyntaxException();
				}, value);
		};
	}
}
