/*******************************
Copyright (c) 2016-2023 Grégoire Angerand

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

#include "EntityPool.h"

namespace yave {
namespace ecs {

usize EntityPool::size() const {
    return _ids.size() - _free.size();
}

bool EntityPool::exists(EntityId id) const {
    return id.is_valid() &&
           id.index() < _ids.size() &&
           _ids[id.index()] == id;
}

EntityId EntityPool::id_from_index(u32 index) const {
    if(index >= _ids.size() || !_ids[index].is_valid()) {
        return EntityId();
    }
    return _ids[index];
}

EntityId EntityPool::create() {
    const EntityId id = [this] {
        if(_free.is_empty()) {
            const usize index = _ids.size();
            _ids.emplace_back(EntityId(u32(index)));
            return _ids.last();
        }

        const u32 index = _free.pop();
        _ids[index].make_valid(index);
        return _ids[index];
    }();

    _parents.set_min_size(id.index() + 1);
    _parents[id.index()] = EntityId();

    return id;
}


void EntityPool::recycle(EntityId id) {
    y_debug_assert(exists(id));
    _ids[id.index()].invalidate();
    _free << id.index();
}

EntityId EntityPool::parent(EntityId id) const {
    if(id.index() < _parents.size()) {
        return _parents[id.index()];
    }
    return {};
}

void EntityPool::set_parent(EntityId id, EntityId parent_id) {
    y_debug_assert(exists(id));

    _parents.set_min_size(id.index() + 1);
    _parents[id.index()] = parent_id;
}


}
}

