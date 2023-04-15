#pragma once
#include "lexer.h"
#include "address.h"
#include "abstree.h"
#include "syntax.h"
#include "ginterface.h"
#include "ring_buffer.h"

namespace byfxxm {
	template <StreamConcept T>
	class Gparser {
	public:
		Gparser(T&& stream) : _syntax(std::move(stream)) {}
		Gparser(const std::string& str) : _syntax(std::istringstream(str)) {}

		void Run(Ginterface& pimpl) {
			while (auto abs_tree = _syntax.Next()) {
				abs_tree.value()(&pimpl);
			}
		}

#if 0
		void AsyncRun(Ginterface& pimpl) {
			RingBuffer<std::optional<Abstree>, 256> fifo;

			auto th = std::thread([&]() {
				while (auto abs_tree = _syntax.Next()) {
					bool break_ = false;
					if (!abs_tree)
						break_ = true;

					while (!fifo.Write(std::move(abs_tree)))
						std::this_thread::yield();

					if (break_)
						break;
				}
				});

			std::optional<Abstree> abs_tree;
			while (1) {
				while (!fifo.Read(abs_tree))
					std::this_thread::yield();

				if (!abs_tree)
					break;

				abs_tree.value()(&pimpl);
			}

			th.join();
		}
#endif

		const Address& Addr() const {
			return _syntax.Addr();
		}

	private:
		Syntax<T> _syntax;
	};

	template <class T>
	Gparser(T) -> Gparser<T>;

	Gparser(std::string)->Gparser<std::istringstream>;
}
