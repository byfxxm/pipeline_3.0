#ifndef _BYFXXM_RING_BUFFER_HPP_
#define _BYFXXM_RING_BUFFER_HPP_

#include <atomic>

namespace byfxxm {
template <class Ty, size_t Num>
  requires(Num > 0)
class RingBuffer {
public:
  bool IsEmpty() const { return _read_index == _write_index; }

  bool IsFull() const { return _Mod(_write_index + 1) == _read_index; }

  bool Write(Ty &&t) {
    if (IsFull())
      return false;

    _data[_write_index] = std::move(t);
    _write_index = _Mod(_write_index + 1);
    return true;
  }

  bool Read(Ty &t) {
    if (IsEmpty())
      return false;

    t = std::move(_data[_read_index]);
    _read_index = _Mod(_read_index + 1);
    return true;
  }

  void Clear() {
    Ty temp;
    while (Read(temp)) {
    }
  }

private:
  size_t _Mod(size_t num) const {
    if constexpr ((Num & (Num - 1)) == 0)
      return (num & (Num - 1));
    else
      return (num % Num);
  }

private:
  std::atomic<size_t> _read_index{0};
  std::atomic<size_t> _write_index{0};
  Ty _data[Num];
};
} // namespace byfxxm

#endif
