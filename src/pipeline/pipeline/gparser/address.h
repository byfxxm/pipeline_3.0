﻿#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include "token.h"

namespace byfxxm {
	class Address {
	public:
		using _Key = double;
		using _Value = UniquePtr<double>;

		_Value& operator[](const _Key& key) {
			if (_dict.find(key) == _dict.end())
				_dict.insert(std::make_pair(key, MakeUnique<double>(nan)));

			return _dict.at(key);
		}

		const _Value& operator[](const _Key& key) const {
			return _dict.at(key);
		}

		bool Contains(const _Key& key) const {
			return _dict.contains(key);
		}

		void Clear() {
			_dict.clear();
		}

	private:
		std::pmr::unordered_map<_Key, _Value> _dict{ &mempool };
	};
}