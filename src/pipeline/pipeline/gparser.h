#pragma once
#include "lexer.h"
#include "address.h"
#include "abstree.h"
#include "syntax.h"
#include "ginterface.h"

namespace byfxxm {
	class Gparser {
	public:
		Gparser(const std::filesystem::path& file) : _syntax(file) {
		}

		Gparser(const std::string& memory) : _syntax(memory) {
		}

		void Run(const Ginterface& pimpl) {
			while (auto abs_tree = _syntax.Next()) {
				abs_tree.value()();
			}
		}

		const Address& GetAddr() const {
			return _syntax.GetAddr();
		}

	private:
		Syntax _syntax;
	};
}
