#pragma once
#include <vector>
#include <functional>
#include <unordered_map>

namespace byfxxm {
	struct Gtag {
		token::Kind code;
		double value;
	};

	using Gparams = std::vector<Gtag>;

	class Address;
	class Ginterface {
	public:
		virtual ~Ginterface() = default;
		virtual bool G0(const Gparams&, const Address&) = 0;
		virtual bool G1(const Gparams&, const Address&) = 0;
		virtual bool G2(const Gparams&, const Address&) = 0;
		virtual bool G3(const Gparams&, const Address&) = 0;
		virtual bool G4(const Gparams&, const Address&) = 0;
	};

	using Gfunc = bool(Ginterface::*)(const Gparams&, const Address&);
}