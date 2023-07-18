#pragma once
#include "array_nd.h"

namespace byfxxm {
	using AxesArray = ArrayNd<double, 1>;
	struct Code {};

	struct Move {
		AxesArray end;
	};

	struct Line {
		AxesArray end;
	};

	struct Arc {
		AxesArray end;
		AxesArray center;
		bool bCcw{ false };
	};
}
