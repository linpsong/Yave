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

#include "EntityWorld.h"

#include <y/utils/log.h>
#include <y/utils/format.h>
#include <y/utils/memory.h>

#include <yave/assets/AssetLoadingContext.h>

#include <numeric>


namespace yave {
namespace ecs2 {

static auto create_component_containers() {
    y_profile();

    core::Vector<std::unique_ptr<ComponentContainerBase>> containers;
    for(const auto* poly_base = ComponentContainerBase::_y_serde3_poly_base.first; poly_base; poly_base = poly_base->next) {
        if(poly_base->create) {
            std::unique_ptr<ComponentContainerBase> container = poly_base->create();
            y_debug_assert(container);

            const ComponentTypeIndex id = container->type_id();
            containers.set_min_size(usize(id) + 1);
            containers[usize(id)] = std::move(container);
        }
    }
    return containers;
}



EntityWorld::EntityWorld() : _containers(create_component_containers()), _matrix(_containers.size()), _system_manager(this) {
    for(auto& container : _containers) {
        if(container) {
            container->_matrix = &_matrix;
        }
    }
}

EntityWorld::~EntityWorld() {
    _containers.clear();
}

const SystemManager& EntityWorld::system_manager() const {
    return _system_manager;
}

SystemManager& EntityWorld::system_manager() {
    return _system_manager;
}

std::string_view EntityWorld::component_type_name(ComponentTypeIndex type_id) const {
    return find_container(type_id)->runtime_info().clean_component_name();
}

usize EntityWorld::entity_count() const {
    return _entities.size();
}

bool EntityWorld::exists(EntityId id) const {
    return _entities.exists(id);
}

EntityId EntityWorld::create_entity() {
    const EntityId id = _entities.create();
    _matrix.add_entity(id);
    return id;
}

void EntityWorld::clear() {
    remove_all_entities();
    _matrix.clear();
    _groups.clear();
}

void EntityWorld::remove_entity(EntityId id) {
    remove_all_components(id);
    remove_all_tags(id);

    _matrix.remove_entity(id);
    _entities.remove(id);
}

void EntityWorld::remove_all_components(EntityId id) {
    y_profile();

    for(auto& container : _containers) {
        if(container) {
            container->remove(id);
        }
    }
}

void EntityWorld::remove_all_tags(EntityId id) {
    y_profile();

    for(const core::String& tag : _matrix.tags()) {
        _matrix.remove_tag(id, tag);
    }
}

void EntityWorld::remove_all_entities() {
    auto cached_entities = core::Vector<EntityId>::from_range(_entities.ids());
    for(const EntityId id : cached_entities) {
        remove_entity(id);
    }
}

const EntityPool& EntityWorld::entity_pool() const {
    return _entities;
}

void EntityWorld::add_tag(EntityId id, std::string_view tag) {
    y_debug_assert(exists(id));
    y_debug_assert(!is_tag_implicit(tag));
    _matrix.add_tag(id, tag);
}

void EntityWorld::remove_tag(EntityId id, std::string_view tag) {
    y_debug_assert(exists(id));
    y_debug_assert(!is_tag_implicit(tag));
    _matrix.remove_tag(id, tag);
}

void EntityWorld::clear_tag(std::string_view tag) {
    y_debug_assert(!is_tag_implicit(tag));
    _matrix.clear_tag(tag);
}

bool EntityWorld::has_tag(EntityId id, std::string_view tag) const {
    y_debug_assert(exists(id));
    y_debug_assert(!is_tag_implicit(tag));
    return _matrix.has_tag(id, tag);
}

core::Span<EntityId> EntityWorld::with_tag(std::string_view tag) const {
    y_debug_assert(!is_tag_implicit(tag));
    return _matrix.with_tag(tag);
}

bool EntityWorld::is_tag_implicit(std::string_view tag) {
    return !tag.empty() && (tag[0] == '@' || tag[0] == '!');
}

EntityId EntityWorld::parent(EntityId id) const {
    return _entities.parent(id);
}

void EntityWorld::set_parent(EntityId id, EntityId parent_id) {
    y_profile();

    _entities.set_parent(id, parent_id);
}

bool EntityWorld::has_parent(EntityId id) const {
    return parent(id).is_valid();
}

bool EntityWorld::has_children(EntityId id) const {
    return _entities.first_child(id).is_valid();
}

bool EntityWorld::is_parent(EntityId id, EntityId parent) const {
    return _entities.is_parent(id, parent);
}

bool EntityWorld::has_component(EntityId id, ComponentTypeIndex type) const {
    y_debug_assert(exists(id));
    return _matrix.has_component(id, type);
}

const ComponentContainerBase* EntityWorld::find_container(ComponentTypeIndex type_id) const {
    y_debug_assert(_containers.size() > usize(type_id));
    return _containers[usize(type_id)].get();
}

ComponentContainerBase* EntityWorld::find_container(ComponentTypeIndex type_id) {
    y_debug_assert(_containers.size() > usize(type_id));
    return _containers[usize(type_id)].get();
}

void EntityWorld::check_exists(EntityId id) const {
    y_always_assert(exists(id), "Entity doesn't exists");
}

void EntityWorld::inspect_components(EntityId id, ComponentInspector* inspector) {
    for(auto& container : _containers) {
        if(container) {
            container->inspect_component(id, inspector);
        }
    }
}

void EntityWorld::register_component_types(System* system) const {
    for(auto& container : _containers) {
        if(container) {
            container->register_component_type(system);
        }
    }
}

serde3::Result EntityWorld::save_state(serde3::WritableArchive& arc) const {
    y_profile();

    y_try(arc.serialize(_entities));
    y_try(arc.serialize(_containers));
    y_try(_matrix.save_tags(arc));

    return core::Ok(serde3::Success::Full);
}

serde3::Result EntityWorld::load_state(serde3::ReadableArchive& arc) {
    clear();

    decltype(_containers) containers;

    y_try(arc.deserialize(_entities));
    y_try(arc.deserialize(containers));
    y_try(_matrix.load_tags(arc));

    for(EntityId id : _entities.ids()) {
        _matrix.add_entity(id);
    }

    for(auto&& container : containers) {
        if(container) {
            container->_matrix = &_matrix;
            container->post_load();

            _containers[usize(container->type_id())] = std::move(container);
        }
    }

    return core::Ok(serde3::Success::Full);
}


}
}
