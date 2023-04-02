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
}