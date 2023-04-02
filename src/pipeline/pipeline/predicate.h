#pragma once
#include <variant>
#include <string>
#include "ginterface.h"

#define IsSameType(lhs, rhs) (std::is_same_v<decltype(lhs), decltype(rhs)>)
#define IsType(v, type) (std::is_same_v<std::remove_cvref_t<decltype(v)>, type>)
#define IsDouble(v) IsType(v, double)
#define IsString(v) IsType(v, std::string)
#define IsDoublePtr(v) IsType(v, double*)

namespace byfxxm {
	using Value = std::variant<double, double*, std::string, Gtag>;

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
		};

		inline auto Neg = [](const Value& value) {
			return std::visit([](auto&& v)->Value {
				if constexpr (IsDouble(v))
					return Value{ -v };
				else
					throw SyntaxException();
				}, value);
		};

		inline auto Pos = [](const Value& value) {
			return std::visit([](auto&& v)->Value {
				if constexpr (IsDouble(v))
					return Value{ v };
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

		struct _GtagHash {
			size_t operator()(const Gtag& tag) const {
				return std::hash<token::Kind>()(tag.code) ^ std::hash<double>()(tag.value);
			}
		};

		struct _GtagEqual {
			bool operator()(const Gtag& tag1, const Gtag& tag2) const {
				return (tag1.code == tag2.code) && (tag1.value == tag2.value);
			}
		};

		inline const std::unordered_map<Gtag, Gfunc, _GtagHash, _GtagEqual> gtag_to_ginterface = {
			{{token::Kind::G, 0}, &Ginterface::G0},
			{{token::Kind::G, 1}, &Ginterface::G1},
			{{token::Kind::G, 2}, &Ginterface::G2},
			{{token::Kind::G, 3}, &Ginterface::G3},
			{{token::Kind::G, 4}, &Ginterface::G4},
		};

		inline auto Gcmd = [](const std::vector<Value>& tags, Ginterface* pimpl, Address& addr)->Value {
			if (tags.empty())
				throw AddressException();

			std::ranges::for_each(tags, [](auto&& ele) {
				if (!std::holds_alternative<Gtag>(ele))
					throw AddressException();

				if (!IsGcode(std::get<Gtag>(ele).code))
					throw AddressException();
				});

			auto& first = std::get<Gtag>(*tags.begin());
			if (!gtag_to_ginterface.contains(first))
				throw AddressException();

			Gparams par;
			std::for_each(tags.begin() + 1, tags.end(), [&](const Value& ele) {
				par.push_back(std::get<Gtag>(ele));
				});

			auto& func = gtag_to_ginterface.at(first);
			if (!(pimpl->*func)(par, addr))
				throw AddressException();

			return first;
		};
	}

	template <class... Ts>
	inline consteval std::variant<std::remove_reference_t<Ts>...> declvariant(Ts&&...) noexcept {
		return {};
	}

	// 一元操作符
	using Unary = decltype(declvariant(
		predicate::Neg
		, predicate::Pos
	));

	// 二元操作符
	using Binary = decltype(declvariant(
		predicate::Plus
		, predicate::Minus
		, predicate::Multi
		, predicate::Div
		, predicate::Assign
	));

	using Sharp = decltype(declvariant(
		predicate::Sharp
	));

	using Gcmd = decltype(declvariant(
		predicate::Gcmd
	));
}
