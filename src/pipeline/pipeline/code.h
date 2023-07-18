#pragma once
#include "array_nd.h"

namespace byfxxm {
	using AxesArray = ArrayNd<double, 1>;
	struct Code {};

	struct Move : Code {
		Move(AxesArray end_) : end(std::move(end_)) {}
		AxesArray end;
	};

	struct Line : Code {
		Line(AxesArray end_) : end(std::move(end_)) {}
		AxesArray end;
	};

	struct Arc : Code {
		AxesArray end;
		AxesArray center;
		bool bCcw{ false };
	};
}
