/*******************************
Copyright (c) 2016-2022 Grégoire Angerand

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

#include "AABBUpdateSystem.h"

#include <yave/components/TransformableComponent.h>

#include <y/utils/log.h>
#include <y/utils/format.h>

namespace yave {

AABBUpdateSystem::AABBUpdateSystem() : ecs::System("AABBUpdateSystem") {
}

void AABBUpdateSystem::setup(ecs::EntityWorld& world) {
    world.make_mutated<TransformableComponent>(world.component_ids<TransformableComponent>());
}

void AABBUpdateSystem::tick(ecs::EntityWorld& world) {
    for(const AABBTypeInfo& info : _infos) {
        for(auto&& [id, comp] : world.query<ecs::Mutate<TransformableComponent>>(world.recently_mutated(info.type))) {
            auto&& [tr] = comp;
            tr.set_aabb(compute_aabb(world, id));
        }
    }
}

AABB AABBUpdateSystem::compute_aabb(const ecs::EntityWorld& world, ecs::EntityId id) const {
    AABB aabb;
    bool init = false;
    for(const AABBTypeInfo& info : _infos) {
        const AABB component_aabb = info.get_aabb(world, id);
        if(!init) {
            init = true;
            aabb = component_aabb;
        } else {
            aabb = aabb.merged(component_aabb);
        }
    }

    return aabb;
}

}
