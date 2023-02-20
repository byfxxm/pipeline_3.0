#pragma once
#include <atomic>
#include <concepts>

namespace byfxxm {
	template <class Ty, size_t Num> requires (Num > 0)
		class RingBuffer {
		public:
			void Reset() {
				read_index_ = write_index_;
			}

			bool IsEmpty() {
				return read_index_ == write_index_;
			}

			bool IsFull() {
				return _Mod(write_index_ + 1) == read_index_;
			}

			template <class T> requires std::same_as<Ty, std::decay_t<T>>
			bool Write(T&& t) {
				if (IsFull())
					return false;

				data_[write_index_] = t;
				write_index_ = _Mod(write_index_ + 1);
				return true;
			}

			template <class T> requires std::same_as<Ty, std::decay_t<T>>
			bool Read(T&& t) {
				if (IsEmpty())
					return false;

				t = data_[read_index_];
				read_index_ = _Mod(read_index_ + 1);
				return true;
			}

		private:
			size_t _Mod(size_t num) {
				if constexpr ((Num & (Num - 1)) == 0)
					return (num & (Num - 1));
				else
					return (num % Num);
			}

		private:
			std::atomic<size_t> read_index_{ 0 };
			std::atomic<size_t> write_index_{ 0 };
			Ty data_[Num];
	};
}
