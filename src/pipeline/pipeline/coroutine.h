#pragma once
#include <coroutine>

NAMESPACE_BEGIN
struct CoTask {
	struct promise_type {
		CoTask get_return_object() { return { std::coroutine_handle<promise_type>::from_promise(*this) }; }
		std::suspend_never initial_suspend() { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void return_void() {}
		void unhandled_exception() {}
		std::suspend_always yield_value(size_t) {
			return {};
		}
	};

	std::coroutine_handle<promise_type> handle;
};

template <class Fn>
struct Awaiter {
	template <class F>
	Awaiter(F&& f) : func(std::forward<F>(f)) {}

	bool await_ready() {
		return false;
	}

	template <class T>
	void await_suspend(T&& t) {
		func(t);
	}

	void await_resume() {
	}

	Fn func;
};

template <class F>
Awaiter(F) -> Awaiter<F>;

NAMESPACE_END
