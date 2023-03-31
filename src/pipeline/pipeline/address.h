#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include "token.h"
//#include "abstree.h"

namespace byfxxm {
	class Address {
	public:
		using K = double;
		using V = std::unique_ptr<double>;

		//decltype(auto) Insert(const K& key, const double* value) {
		//	return _dict.insert(std::make_pair(key, std::make_unique<double>(value)));
		//}

		V& operator[](const K& key) {
			if (_dict.find(key) == _dict.end())
				_dict.insert(std::make_pair(key, std::make_unique<double>()));

			return _dict.at(key);
		}

		bool Find(const K& key) const {
			return _dict.find(key) != _dict.end();
		}

		void Clear() {
			_dict.clear();
		}

	private:
		std::unordered_map<K, V> _dict;
	};
}
