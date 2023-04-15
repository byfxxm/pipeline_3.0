#pragma once
#include <memory>
#include <type_traits>

namespace byfxxm {
	template <class T>
	class ClonePtr {
	public:
		ClonePtr() = default;
		~ClonePtr() = default;
		ClonePtr(ClonePtr&&) noexcept = default;
		ClonePtr& operator=(ClonePtr&&) noexcept = default;

		template <class D>
		ClonePtr(std::unique_ptr<D>&& rhs) : _pointer(std::move(rhs)) {}

		ClonePtr(const std::unique_ptr<T>& rhs) {
			if constexpr (std::is_abstract_v<T>)
				_pointer = rhs->Clone();
			else
				_pointer = std::make_unique<T>(*rhs);
		}

		ClonePtr(const ClonePtr& rhs) : ClonePtr(rhs._pointer) {}

		// copy and swap
		ClonePtr& operator=(const ClonePtr& rhs) {
			ClonePtr copy(rhs);
			swap(*this, copy);
			return *this;
		}

		operator bool() const noexcept {
			return _pointer.operator bool();
		}

		decltype(auto) operator->() const noexcept {
			return _pointer.operator->();
		}

		decltype(auto) reset(auto&&... pars) noexcept {
			return _pointer.reset(std::forward<decltype(pars)>(pars)...);
		}

		decltype(auto) get() const noexcept {
			return _pointer.get();
		}

		friend void swap(ClonePtr& lhs, ClonePtr& rhs) noexcept {
			std::swap(lhs._pointer, rhs._pointer);
		}

	private:
		std::unique_ptr<T> _pointer;
	};

	template <class T>
	ClonePtr(std::unique_ptr<T>) -> ClonePtr<T>;
}