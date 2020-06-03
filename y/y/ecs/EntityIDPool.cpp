/*******************************
Copyright (c) 2016-2020 Gr�goire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/

#include "EntityIDPool.h"

namespace y {
namespace ecs {

usize EntityIDPool::size() const {
	return _ids.size() - _free.size();
}

bool EntityIDPool::contains(EntityID id) const {
	return id.is_valid() &&
		   id.index() < _ids.size() &&
		   _ids[id.index()] == id;
}

EntityID EntityIDPool::create() {
	if(_free.is_empty()) {
		const usize index = _ids.size();
		_ids.emplace_back(EntityID(index));
		return _ids.last();
	}

	const u32 index = _free.pop();
	_ids[index].make_valid(index);
	return _ids[index];
}


void EntityIDPool::recycle(EntityID id) {
	y_debug_assert(contains(id));
	_ids[id.index()].invalidate();
	_free << id.index();
}


}
}