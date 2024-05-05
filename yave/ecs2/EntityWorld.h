/*******************************
Copyright (c) 2016-2024 Grégoire Angerand

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
#ifndef YAVE_ECS2_ENTITYWORLD_H
#define YAVE_ECS2_ENTITYWORLD_H

#include <yave/ecs/tags.h>

#include "SystemManager.h"
#include "ComponentContainer.h"
#include "EntityGroup.h"
#include "traits.h"

#include <yave/ecs/EntityPool.h>
#include <yave/ecs/EntityPrefab.h>

#include <y/utils/log.h>

namespace yave {
namespace ecs2 {

using EntityPool = ecs::EntityPool;
using EntityPrefab = ecs::EntityPrefab;

class EntityWorld : NonMovable {

    public:
        EntityWorld();
        ~EntityWorld();


        const SystemManager& system_manager() const;
        SystemManager& system_manager();


        std::string_view component_type_name(ComponentTypeIndex type_id) const;


        usize entity_count() const;
        bool exists(EntityId id) const;

        EntityId create_entity();
        EntityId create_entity(const EntityPrefab&) { y_fatal("FIXME"); }

        void clear();

        void remove_entity(EntityId id);
        void remove_all_components(EntityId id);
        void remove_all_tags(EntityId id);

        void remove_all_entities();

        const EntityPool& entity_pool() const;



        // ---------------------------------------- Tags ----------------------------------------

        void add_tag(EntityId id, std::string_view tag);
        void remove_tag(EntityId id, std::string_view tag);
        void clear_tag(std::string_view tag);
        bool has_tag(EntityId id, std::string_view tag) const;

        core::Span<EntityId> with_tag(std::string_view tag) const;

        static bool is_tag_implicit(std::string_view tag);




        // ---------------------------------------- Parent ----------------------------------------

        EntityId parent(EntityId id) const;
        void set_parent(EntityId id, EntityId parent_id);

        bool has_parent(EntityId id) const;
        bool has_children(EntityId id) const;

        bool is_parent(EntityId id, EntityId parent) const;

        auto parents(EntityId id) const {
            return _entities.parents(id);
        }

        auto children(EntityId id) const {
            return _entities.children(id);
        }



        // ---------------------------------------- Components ----------------------------------------

        bool has_component(EntityId id, ComponentTypeIndex type) const;

        template<typename T>
        const T* component(EntityId id) const {
            return find_container<T>()->try_get(id);
        }

        template<typename T>
        T* component_mut(EntityId id) {
            return find_container<T>()->try_get_mut(id);
        }

        template<typename T>
        bool has_component(EntityId id) const {
            return has_component(id, type_index<T>());
        }

        template<typename T>
        const SparseComponentSet<T>& component_set() const {
            return find_container<T>()->component_set();
        }



        template<typename T>
        T* get_or_add_component(EntityId id) {
            return find_container<T>()->get_or_add(id);
        }

        template<typename T, typename... Args>
        T* add_or_replace_component(EntityId id, Args&&... args) {
            check_exists(id);
            return find_container<T>()->add_or_replace(id, y_fwd(args)...);
        }




        // ---------------------------------------- Systems ----------------------------------------

        template<typename S, typename... Args>
        S* add_system(Args&&... args) {
            S* system = _system_manager.add_system<S>(y_fwd(args)...);
            register_component_types(system);
            return system;
        }

        template<typename S>
        S* find_system() {
            return _system_manager.find_system<S>();
        }





        // ---------------------------------------- Groups ----------------------------------------

        template<typename... Ts>
        const EntityGroup<Ts...>& create_group(core::Span<std::string_view> tags = {}) {
            y_profile();
            using group_type = EntityGroup<Ts...>;
            for(const auto& group : _groups) {
                if(const auto* typed_group = dynamic_cast<group_type*>(group.get())) {
                    if(tags.size() != typed_group->tags().size()) {
                        continue;
                    }
                    if(!std::equal(tags.begin(), tags.end(), typed_group->tags().begin())) {
                        continue;
                    }
                    return *typed_group;
                } else if(group->types().size() == sizeof...(Ts)) {
                    if((group->contains<Ts>() && ...)) {
                        log_msg("An entity group with similar component set already exists", Log::Warning);
                    }
                }
            }
            return *create_new_group<Ts...>(tags);
        }


        template<typename... Ts>
        const EntityGroup<Ts...>& create_group(core::Span<std::string_view> tags = {}) const {
            static_assert(EntityGroup<Ts...>::is_const);
            return const_cast<EntityWorld*>(this)->create_group<Ts...>(tags);
        }







        // ---------------------------------------- Misc ----------------------------------------

        void inspect_components(EntityId id, ComponentInspector* inspector);

        serde3::Result save_state(serde3::WritableArchive& arc) const;
        serde3::Result load_state(serde3::ReadableArchive& arc);



    private:
        const ComponentContainerBase* find_container(ComponentTypeIndex type_id) const;
        ComponentContainerBase* find_container(ComponentTypeIndex type_id);

        void register_component_types(System* system) const;

        void check_exists(EntityId id) const;


        core::Vector<std::unique_ptr<ComponentContainerBase>> _containers;
        core::Vector<std::unique_ptr<EntityGroupBase>> _groups;

        ComponentMatrix _matrix;
        EntityPool _entities;

        SystemManager _system_manager;


    private:
        template<typename T>
        friend class ComponentContainer;

        template<typename T>
        const ComponentContainer<T>* find_container() const {
            static_assert(std::is_same_v<traits::component_raw_type_t<T>, T>);
            static const auto static_info = ComponentRuntimeInfo::create<T>();
            unused(static_info);
            return static_cast<const ComponentContainer<T>*>(find_container(type_index<T>()));
        }

        template<typename T>
        ComponentContainer<T>* find_container() {
            static_assert(std::is_same_v<traits::component_raw_type_t<T>, T>);
            static const auto static_info = ComponentRuntimeInfo::create<T>();
            unused(static_info);
            return static_cast<ComponentContainer<T>*>(find_container(type_index<T>()));
        }

        template<typename... Ts>
        EntityGroup<Ts...>* create_new_group(core::Span<std::string_view> tags) {
            auto group = std::make_unique<EntityGroup<Ts...>>(std::tuple{find_container<traits::component_raw_type_t<Ts>>()...}, tags);
            auto* group_ptr = group.get();
            _groups.emplace_back(std::move(group));
            _matrix.register_group(group_ptr);
            return group_ptr;
        }
};

}
}

#include "EntityWorld.inl"
#include <yave/ecs/EntityWorld.inl>

#endif // YAVE_ECS2_ENTITYWORLD_H
