#pragma once
#include "token.h"
#include "syntax.h"
#include "ginterface.h"

namespace byfxxm {
	template <StreamConcept T>
	class Gparser {
	public:
		Gparser(T&& stream, Ginterface* pimpl = nullptr) : _syntax(std::move(stream), pimpl) {}
		Gparser(const std::string& str, Ginterface* pimpl = nullptr) : _syntax(std::istringstream(str), pimpl) {}
		Gparser(const std::filesystem::path& file, Ginterface* pimpl = nullptr) : _syntax(std::ifstream(file), pimpl) {}

		void Run() {
			while (auto abs_tree = _syntax.Next()) {
				abs_tree.value().Execute();
			}
		}

		const Address& Addr() const {
			return _syntax.Addr();
		}

	private:
		Syntax<T> _syntax;
	};

	template <class T>
	Gparser(T, Ginterface*) -> Gparser<T>;

	Gparser(std::string, Ginterface*)->Gparser<std::istringstream>;
	Gparser(std::filesystem::path, Ginterface*)->Gparser<std::ifstream>;
}
