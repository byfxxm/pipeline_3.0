#pragma once
#include <memory>
#include <string>
#include <format>
#include <algorithm>
#include <iostream>
#include "worker.h"
#include "code.h"

namespace byfxxm {
	std::string _Format(const AxesArray& axes) {
		std::string ret;
		std::ranges::for_each(axes, [&](auto&& item) {
			ret += " ";
			ret += std::to_string(item);
			});

		return ret;
	}

	class Issuer : private Worker {
	private:
		virtual bool Do(std::unique_ptr<Code> code, const WriteFunc& writefn) noexcept override {
			switch (code->tag) {
			case codetag::MOVE:
				std::cout << "G0" << _Format(static_cast<Move*>(code.get())->end) << std::endl;
				break;
			case codetag::LINE:
				std::cout << "G1" << _Format(static_cast<Line*>(code.get())->end) << std::endl;
				break;
			case codetag::ARC:
				break;
			default:
				break;
			}

			return true;
		}
	};
}
