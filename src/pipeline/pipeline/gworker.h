#pragma once
#include <type_traits>
#include "worker.h"
#include "gparser/gparser.h"
#include "code.h"

namespace byfxxm {
	inline void print_gparams(std::string str, const byfxxm::Gparams& params) {
		static const std::unordered_map<token::Kind, std::string> map = {
			{token::Kind::X, "X"},
			{token::Kind::Y, "Y"},
			{token::Kind::Z, "Z"},
		};

		std::ranges::for_each(params, [&](const Gtag& item) {
			str += std::format(" {}{}", map.at(item.code), item.value);
			});
		puts(str.c_str());
	}

	inline AxesArray GparamsToAxesArray(const byfxxm::Gparams& params) {
		AxesArray ret(6);
		ret.Memset(nan);
		std::ranges::for_each(params, [&](const Gtag& item) {
			switch (item.code) {
			case token::Kind::X:
				ret[0] = item.value;
				break;
			case token::Kind::Y:
				ret[1] = item.value;
				break;
			case token::Kind::Z:
				ret[2] = item.value;
				break;
			default:
				break;
			}
			});

		return ret;
	}

	class Gpimpl : public byfxxm::Ginterface {
	public:
		Gpimpl(const WriteFunc& writefn) : _writefn(writefn) {}

		virtual bool None(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
			std::string str;
			if (_last == Gtag{token::Kind::G, 0})
				_writefn(std::make_unique<Move>(GparamsToAxesArray(params)));
			else if (_last == Gtag{token::Kind::G, 1})
				_writefn(std::make_unique<Line>(GparamsToAxesArray(params)));
			else if (_last == Gtag{token::Kind::G, 2})
				str = "G2";
			else if (_last == Gtag{token::Kind::G, 3})
				str = "G3";

			return true;
		}

		virtual bool G0(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
			_last = { token::Kind::G, 0 };
			_writefn(std::make_unique<Move>(GparamsToAxesArray(params)));
			return true;
		}

		virtual bool G1(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
			_last = { token::Kind::G, 1 };
			_writefn(std::make_unique<Line>(GparamsToAxesArray(params)));
			return true;
		}

		virtual bool G2(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
			_last = { token::Kind::G, 2 };
			print_gparams("G2", params);
			return true;
		}

		virtual bool G3(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
			_last = { token::Kind::G, 3 };
			print_gparams("G3", params);
			return true;
		}

		virtual bool G4(const byfxxm::Gparams& params, const byfxxm::Address* addr) override {
			print_gparams("G4", params);
			return true;
		}

	private:
		Gtag _last{ token::Kind::G, 0 };
		WriteFunc _writefn;
	};

	class Gworker : private Worker {
	public:
		Gworker(StreamConcept auto&& stream) : _parser(std::forward<decltype(stream)>(stream)) {}

	private:
		virtual bool Do(std::unique_ptr<Code>, const WriteFunc& writefn) noexcept override {
			std::visit([&](auto&& parser) {
				if constexpr (std::is_same_v<std::monostate, std::decay_t<decltype(parser)>>)
					assert(0);
				else {
					Address addr;
					Gpimpl gpimpl(writefn);
					parser.Run(&addr, &gpimpl);
				}
				}, _parser);
			return true;
		}

	private:
		std::variant<std::monostate, Gparser<std::fstream>, Gparser<std::stringstream>> _parser;
	};
}
