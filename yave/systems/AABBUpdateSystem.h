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
#ifndef YAVE_SYSTEMS_AABBUPDATESYSTEM_H
#define YAVE_SYSTEMS_AABBUPDATESYSTEM_H

#include <yave/ecs/EntityWorld.h>
#include <yave/meshes/AABB.h>

#include <y/core/Vector.h>


namespace yave {

class AABBUpdateSystem : public ecs::System {
    public:
        AABBUpdateSystem();

        void setup(ecs::EntityWorld& world) override;
        void tick(ecs::EntityWorld& world) override;

        template<typename T>
        void register_component_type() {
            _infos << AABBTypeInfo {
                [](const ecs::EntityWorld& world, ecs::EntityId id) -> AABB {
                    if(const T* comp = world.component<T>(id)) {
                        return comp->aabb();
                    }
                    return AABB();
                },
                ecs::type_index<T>(),
            };
        }

    private:
        struct AABBTypeInfo {
            AABB (*get_aabb)(const ecs::EntityWorld&, ecs::EntityId);
            ecs::ComponentTypeIndex type;
        };

        AABB compute_aabb(const ecs::EntityWorld&world, ecs::EntityId id) const;


        core::Vector<AABBTypeInfo> _infos;
};

}

#endif // YAVE_SYSTEMS_AABBUPDATESYSTEM_H

