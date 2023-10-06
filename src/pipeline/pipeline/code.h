#pragma once
#include "array_nd.h"

namespace byfxxm {
using AxesArray = ArrayNd<double, 1>;

enum class codetag {
  NA,
  MOVE,
  LINE,
  ARC,
};

struct Code {
  codetag tag{codetag::NA};
};

struct Move : Code {
  Move(AxesArray end_) : Code{codetag::MOVE}, end(std::move(end_)) {}
  AxesArray end;
};

struct Line : Code {
  Line(AxesArray end_) : Code{codetag::LINE}, end(std::move(end_)) {}
  AxesArray end;
};

struct Arc : Code {
  Arc(AxesArray end_, AxesArray center_, bool ccw_)
      : Code{codetag::ARC}, end(std::move(end_)), center(std::move(center_)),
        ccw(ccw_) {}
  AxesArray end;
  AxesArray center;
  bool ccw{false};
};
} // namespace byfxxm
