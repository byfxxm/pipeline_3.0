#pragma once
#include <memory>
#include <type_traits>
#include <memory_resource>

namespace byfxxm
{
	template <class T>
	struct Deleter
	{
		Deleter() = default;

		template <std::derived_from<T> T2>
		Deleter(const Deleter<T2> &) noexcept : _size(sizeof(T2)) {}

		void operator()(T *p) const
		{
			delete p;
		}

		size_t _size{sizeof(T)};
	};

	template <class T, class D = Deleter<T>>
	class UniquePtr
	{
	public:
		UniquePtr() = default;

		template <class T2>
		UniquePtr(UniquePtr<T2> &&rhs) noexcept : _pointer(std::move(rhs._pointer)) {}

		template <class T2>
		UniquePtr(T2 *rhs) noexcept : _pointer(rhs) {}

		explicit operator bool() const noexcept
		{
			return _pointer.operator bool();
		}

		decltype(auto) operator->() const noexcept
		{
			return _pointer.operator->();
		}

		decltype(auto) Reset(auto &&...pars) noexcept
		{
			return _pointer.reset(std::forward<decltype(pars)>(pars)...);
		}

		decltype(auto) Get() const noexcept
		{
			return _pointer.get();
		}

		decltype(auto) operator*() const noexcept
		{
			return _pointer.operator*();
		}

	private:
		std::unique_ptr<T, D> _pointer;

		template <class T2, class D2>
		friend class UniquePtr;
	};

	template <class T, class... Args>
	[[nodiscard]] auto MakeUnique(Args &&...args) noexcept
	{
		return UniquePtr<T>(new T(std::forward<Args>(args)...));
	}

	template <class T>
	class ClonePtr
	{
	public:
		ClonePtr() = default;
		~ClonePtr() = default;
		ClonePtr(ClonePtr &&) noexcept = default;
		ClonePtr &operator=(ClonePtr &&) noexcept = default;

		template <class T2>
		ClonePtr(UniquePtr<T2> &&rhs) noexcept : _pointer(std::move(rhs)) {}

		ClonePtr(const UniquePtr<T> &rhs)
		{
			if constexpr (std::is_abstract_v<T>)
				_pointer = rhs->Clone();
			else
				_pointer = MakeUnique<T>(*rhs);
		}

		ClonePtr(const ClonePtr &rhs) : ClonePtr(rhs._pointer) {}

		// copy and swap
		ClonePtr &operator=(const ClonePtr &rhs)
		{
			using std::swap;
			ClonePtr copy(rhs);
			swap(*this, copy);
			return *this;
		}

		explicit operator bool() const noexcept
		{
			return _pointer.operator bool();
		}

		decltype(auto) operator->() const noexcept
		{
			return _pointer.operator->();
		}

		decltype(auto) Reset(auto &&...pars) noexcept
		{
			return _pointer.Reset(std::forward<decltype(pars)>(pars)...);
		}

		decltype(auto) Get() const noexcept
		{
			return _pointer.Get();
		}

	private:
		UniquePtr<T> _pointer;
	};
}
