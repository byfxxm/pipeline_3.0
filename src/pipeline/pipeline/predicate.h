#pragma once
#include <variant>
#include <string>
#include "ginterface.h"

#define IsSameType(lhs, rhs) (std::is_same_v<decltype(lhs), decltype(rhs)>)
#define IsType(v, type) (std::is_same_v<std::remove_cvref_t<decltype(v)>, type>)
#define IsDouble(v) IsType(v, double)
#define IsString(v) IsType(v, std::string)
#define IsDoublePtr(v) IsType(v, double*)
#define IsGroup(v) IsType(v, Group)

namespace byfxxm {
	using Group = std::vector<double>;
	using Value = std::variant<std::monostate, double, double*, std::string, bool, Gtag, Group>;

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
					throw SyntaxException("plus error");
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
					throw SyntaxException("minus error");
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
					throw SyntaxException("multiple error");
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
					throw SyntaxException("divide error");
				}, lhs, rhs);
		};

		inline auto Assign = [](Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					*l = *r;
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					*l = r;
				else
					throw SyntaxException("assign error");

				return Value{ l };
				}, lhs, rhs);
		};

		inline auto Neg = [](const Value& value) {
			return std::visit([](auto&& v)->Value {
				if constexpr (IsDouble(v))
					return Value{ -v };
				else if constexpr (IsDoublePtr(v))
					return Value{ -*v };
				else
					throw SyntaxException("negative error");
				}, value);
		};

		inline auto Pos = [](const Value& value) {
			return std::visit([](auto&& v)->Value {
				if constexpr (IsDouble(v))
					return Value{ v };
				else if constexpr (IsDoublePtr(v))
					return Value{ *v };
				else
					throw SyntaxException("positive error");
				}, value);
		};

		inline auto Sharp = [](const Value& value, Address& addr) {
			return std::visit([&](auto&& v)->Value {
				if constexpr (IsDouble(v))
					return addr[v].get();
				else if constexpr (IsDoublePtr(v))
					return addr[*v].get();
				else
					throw SyntaxException("sharp error");
				}, value);
		};

		inline auto GT = [](const Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					return Value{ *l > *r };
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					return Value{ *l > r };
				else if constexpr (IsDouble(l) && IsDoublePtr(r))
					return Value{ l > *r };
				else if constexpr (IsDouble(l) && IsDouble(r))
					return Value{ l > r };
				else
					throw SyntaxException();
				}, lhs, rhs);
		};

		inline auto GE = [](const Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					return Value{ *l >= *r };
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					return Value{ *l >= r };
				else if constexpr (IsDouble(l) && IsDoublePtr(r))
					return Value{ l >= *r };
				else if constexpr (IsDouble(l) && IsDouble(r))
					return Value{ l >= r };
				else
					throw SyntaxException();
				}, lhs, rhs);
		};

		inline auto LT = [](const Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					return Value{ *l < *r };
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					return Value{ *l < r };
				else if constexpr (IsDouble(l) && IsDoublePtr(r))
					return Value{ l < *r };
				else if constexpr (IsDouble(l) && IsDouble(r))
					return Value{ l < r };
				else
					throw SyntaxException();
				}, lhs, rhs);
		};

		inline auto LE = [](const Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					return Value{ *l <= *r };
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					return Value{ *l <= r };
				else if constexpr (IsDouble(l) && IsDoublePtr(r))
					return Value{ l <= *r };
				else if constexpr (IsDouble(l) && IsDouble(r))
					return Value{ l <= r };
				else
					throw SyntaxException();
				}, lhs, rhs);
		};

		inline auto EQ = [](const Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					return Value{ *l == *r };
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					return Value{ *l == r };
				else if constexpr (IsDouble(l) && IsDoublePtr(r))
					return Value{ l == *r };
				else if constexpr (IsDouble(l) && IsDouble(r))
					return Value{ l == r };
				else
					throw SyntaxException();
				}, lhs, rhs);
		};

		inline auto NE = [](const Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					return Value{ *l != *r };
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					return Value{ *l != r };
				else if constexpr (IsDouble(l) && IsDoublePtr(r))
					return Value{ l != *r };
				else if constexpr (IsDouble(l) && IsDouble(r))
					return Value{ l != r };
				else
					throw SyntaxException();
				}, lhs, rhs);
		};

		inline auto Gcmd = [](const std::vector<Value>& tags, Ginterface* pimpl, Address& addr)->Value {
			if (tags.empty())
				throw AbstreeException();

			std::ranges::for_each(tags, [](auto&& ele) {
				if (!std::holds_alternative<Gtag>(ele))
					throw AbstreeException();

				if (!IsGcode(std::get<Gtag>(ele).code))
					throw AbstreeException();
				});

			auto& first = std::get<Gtag>(*tags.begin());
			if (!gtag_to_ginterface.contains(first))
				throw AbstreeException();

			Gparams par;
			std::for_each(tags.begin() + 1, tags.end(), [&](const Value& ele) {
				auto& tag = std::get<Gtag>(ele);
				if (IsNaN(tag.value))
					return;

				par.push_back(std::get<Gtag>(ele));
				});

			if (!gtag_to_ginterface.contains(first))
				throw AbstreeException();

			auto& func = gtag_to_ginterface.at(first);
			if (!(pimpl->*func)(par, addr))
				throw AbstreeException();

			return std::monostate{};
		};

		template <token::Kind K>
		inline auto Gcode = [](const Value& value) {
			return std::visit([&](auto&& v)->Value {
				if constexpr (IsDoublePtr(v))
					return Gtag{ K, *v };
				else if constexpr (IsDouble(v))
					return Gtag{ K, v };
				else
					throw SyntaxException("gcode error");
				}, value);
		};

		inline auto Comma = [](Value& lhs, const Value& rhs) {
			return std::visit([](auto&& l, auto&& r)->Value {
				if constexpr (IsGroup(l) && IsDouble(r)) {
					l.push_back(r);
					return l;
				}
				else if constexpr (IsGroup(l) && IsDoublePtr(r)) {
					l.push_back(*r);
					return l;
				}
				else if constexpr (IsDouble(l) && IsDouble(r))
					return Group{ l, r };
				else if constexpr (IsDouble(l) && IsDoublePtr(r))
					return Group{ l, *r };
				else if constexpr (IsDoublePtr(l) && IsDouble(r))
					return Group{ *l, r };
				else if constexpr (IsDoublePtr(l) && IsDoublePtr(r))
					return Group{ *l, *r };
				else
					throw SyntaxException("comma error");
				}, lhs, rhs);
		};

		inline auto Max = [](Value& value)->Value {
			return std::visit([](auto&& v)->Value {
				if constexpr (IsGroup(v)) {
					return *std::ranges::max_element(v, [](double lhs, double rhs) {
						return lhs < rhs;
						});
				}
				else if constexpr (IsDouble(v))
					return v;
				else if constexpr (IsDoublePtr(v))
					return *v;
				else
					throw SyntaxException("max error");
				}, value);
		};

		inline auto Min = [](Value& value) {
			return std::visit([](auto&& v)->Value {
				if constexpr (IsGroup(v)) {
					return *std::ranges::min_element(v, [](double lhs, double rhs) {
						return lhs < rhs;
						});
				}
				else if constexpr (IsDouble(v))
					return v;
				else if constexpr (IsDoublePtr(v))
					return *v;
				else
					throw SyntaxException("min error");
				}, value);
		};
	}

	template <class... Ts>
	inline consteval std::variant<std::remove_reference_t<Ts>...> ToVariant(Ts&&...) noexcept {
		return {};
	}

	// 一元操作符
	using Unary = decltype(ToVariant(
		predicate::Neg
		, predicate::Pos
		, predicate::Gcode<token::Kind::G>
		, predicate::Gcode<token::Kind::M>
		, predicate::Gcode<token::Kind::X>
		, predicate::Gcode<token::Kind::Y>
		, predicate::Gcode<token::Kind::Z>
		, predicate::Gcode<token::Kind::A>
		, predicate::Gcode<token::Kind::B>
		, predicate::Gcode<token::Kind::C>
		, predicate::Gcode<token::Kind::I>
		, predicate::Gcode<token::Kind::J>
		, predicate::Gcode<token::Kind::K>
		, predicate::Max
		, predicate::Min
	));

	// 二元操作符
	using Binary = decltype(ToVariant(
		predicate::Plus
		, predicate::Minus
		, predicate::Multi
		, predicate::Div
		, predicate::Assign
		, predicate::GT
		, predicate::GE
		, predicate::LT
		, predicate::LE
		, predicate::EQ
		, predicate::NE
		, predicate::Comma
	));

	using Sharp = decltype(ToVariant(
		predicate::Sharp
	));

	using Gcmd = decltype(ToVariant(
		predicate::Gcmd
	));

	// 定义谓词
	using Predicate = std::variant<Value, Unary, Binary, Sharp, Gcmd>;
}
