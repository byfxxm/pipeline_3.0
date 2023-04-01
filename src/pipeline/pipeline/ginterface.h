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

	using Gparam = std::vector<Gtag>;

	struct Gcmd {
		Gtag cmd;
		Gparam prams;
	};

	using Gfunc = std::function<bool(const Gparam&)>;

	class Ginterface {
	public:
		virtual ~Ginterface() = default;
		virtual bool G0(const Gfunc&) = 0;
	};
}