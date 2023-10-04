#pragma once
#include <vector>

namespace byfxxm {
	using Gparams = std::vector<Gtag>;
	class Address;

	class Ginterface {
	public:
		virtual ~Ginterface() = default;
		virtual bool None(const Gparams&, const Address*) = 0;
		virtual bool G0(const Gparams&, const Address*) = 0;
		virtual bool G1(const Gparams&, const Address*) = 0;
		virtual bool G2(const Gparams&, const Address*) = 0;
		virtual bool G3(const Gparams&, const Address*) = 0;
		virtual bool G4(const Gparams&, const Address*) = 0;
	};
}
