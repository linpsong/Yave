/*******************************
Copyright (C) 2009-2010 grégoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/

#ifndef N_CORE_PAIR_H
#define N_CORE_PAIR_H

#include "Array.h"

namespace n {
namespace core {

template<typename T, typename U = T>
class Pair
{
	public:
		Pair(const T &a, const U&b) : _1(a), _2(b) {
		}

		bool operator<(const Pair<T, U> &p) const {
			return _1 == p._1 ? _2 < p._2 : _1 < p._1;
		}

		bool operator>(const Pair<T, U> &p) const {
			return _1 == p._1 ? _2 > p._2 : _1 > p._1;
		}

		bool operator==(const Pair<T, U> &p) const {
			return _1 == p._1 && _2 == p._2;
		}

		bool operator!=(const Pair<T, U> &p) const {
			return _1 != p._1 || _2 != p._2;
		}

		const Pair<T, U> &operator=(const Pair<T, U> &p) {
			_1 = p._1;
			_2 = p._2;
			return *this;
		}

		operator std::pair<T, U>() const {
			return std::pair<T, U>(_1, _2);
		}

		Array<T> toArray() const {
			return Array<T>(_1, _2);
		}

		T _1;
		U _2;
};

} //core
} //n


#endif // Pair_H
