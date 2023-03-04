#pragma once

namespace byfxxm {
	class Space {
	public:
		static constexpr char space[] = {
			' ',
			'\t',
			'\n',
			'\r',
		};

		static bool IsSpace(char ch) {
			for (auto c : space) {
				if (c == ch)
					return true;
			}

			return false;
		}
	};
}