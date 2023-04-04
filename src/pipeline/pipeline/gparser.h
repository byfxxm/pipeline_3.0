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

		void Run(Ginterface& pimpl) {
			//try {
				while (auto abs_tree = _syntax.Next()) {
					abs_tree.value()(&pimpl);
				}
			//}
			//catch (const ParseException&) {
			//	throw;
			//}
		}

		const Address& Addr() const {
			return _syntax.Addr();
		}

	private:
		Syntax _syntax;
	};
}
