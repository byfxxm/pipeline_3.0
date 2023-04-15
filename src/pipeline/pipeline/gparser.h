#pragma once
#include "token.h"
#include "syntax.h"
#include "ginterface.h"

namespace byfxxm {
	template <StreamConcept T>
	class Gparser {
	public:
		Gparser(T&& stream) : _syntax(std::move(stream)) {}
		Gparser(const std::string& str) : _syntax(std::istringstream(str)) {}
		Gparser(const std::filesystem::path& file) : _syntax(std::ifstream(file)) {}

		void Run(Ginterface& pimpl) {
			while (auto abs_tree = _syntax.Next()) {
				abs_tree.value()(&pimpl);
			}
		}

		const Address& Addr() const {
			return _syntax.Addr();
		}

	private:
		Syntax<T> _syntax;
	};

	template <class T>
	Gparser(T) -> Gparser<T>;

	Gparser(std::string)->Gparser<std::istringstream>;
	Gparser(std::filesystem::path)->Gparser<std::ifstream>;
}
