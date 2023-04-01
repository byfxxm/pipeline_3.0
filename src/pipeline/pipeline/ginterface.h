#pragma once
#include <vector>
#include <functional>
#include <unordered_map>

namespace byfxxm {
	enum class Gcode {
		G,
		M,
		X,
		Y,
		Z,
		A,
		B,
		C,
	};

	struct Gtag {
		Gcode code;
		double value;
	};

	using Gparams = std::vector<Gtag>;

	class Ginterface {
	public:
		virtual ~Ginterface() = default;
		virtual bool G0(const Gparams&, const Address&) = 0;
	};
}