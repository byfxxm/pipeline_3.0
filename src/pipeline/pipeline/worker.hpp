#ifndef _BYFXXM_WORKER_HPP_
#define _BYFXXM_WORKER_HPP_

#include <functional>
#include <memory>

namespace byfxxm {
struct Code;
using WriteFunc = std::function<void(std::unique_ptr<Code>)>;

class Worker {
public:
  virtual ~Worker() = default;
  virtual bool Do(std::unique_ptr<Code>, const WriteFunc &) noexcept = 0;
};
} // namespace byfxxm

#endif
