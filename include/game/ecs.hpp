#pragma once

// Header files include //
#include "pipeline/model.hpp"
#include "logger/logging.hpp"
#include "components/transform2dComponent.hpp"
#include "components/colorComponent.hpp"
#include "components/modelComponent.hpp"
#include "components/velocityComponent.hpp"

// STD include //
#include <stdexcept>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <limits>
#include <algorithm>
#include <tuple>
#include <utility>
#include <cassert>

namespace vulkan {

    using Entity = uint32_t;
    static constexpr Entity INVALID_ENTITY = std::numeric_limits<Entity>::max();
    
    class IComponentStorage {
        public:
            virtual ~IComponentStorage() = default;
            virtual void remove(Entity entity) = 0;
            virtual bool contains(Entity entity) const = 0;
            virtual std::vector<Entity> entities() const = 0;
    };
    
    template<typename Component>
    class ComponentStorage : public IComponentStorage {
        private:
            std::unordered_map<Entity, Component> _components;
        public:
            template<typename... Args>
            Component& emplace(Entity entity, Args&&... args) {
                typename std::unordered_map<Entity, Component>::iterator iterator = _components.find(entity);
                if (iterator == _components.end()) {
                    std::pair<typename std::unordered_map<Entity, Component>::iterator, bool> ins =
                        _components.emplace(entity, Component(std::forward<Args>(args)...));
                    return ins.first->second;
                } else {
                    iterator->second = Component(std::forward<Args>(args)...);
                    return iterator->second;
                }
            }
        
            void remove(Entity entity) override {
                _components.erase(entity);
            }
        
            Component* try_get(Entity entity) {
                typename std::unordered_map<Entity, Component>::iterator iterator = _components.find(entity);
                if (iterator == _components.end()) {
                    return nullptr;
                }
                return &iterator->second;
            }
        
            const Component* try_get(Entity entity) const {
                typename std::unordered_map<Entity, Component>::const_iterator iterator = _components.find(entity);
                if (iterator == _components.end()) {
                    return nullptr;
                }
                return &iterator->second;
            }
        
            bool contains(Entity entity) const override {
                return _components.find(entity) != _components.end();
            }
        
            std::vector<Entity> entities() const override {
                std::vector<Entity> out;
                out.reserve(_components.size());
                for (const std::pair<const Entity, Component>& component : _components) {
                    out.push_back(component.first);
                }
                return out;
            }
        
            const std::unordered_map<Entity, Component>& map() const {
                return _components;
            }
        };
    
    class Registry {
        private:
            Entity _nextId{0};
            std::vector<Entity> _alive;    
            std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> _storages;
        
            template<typename Component>
            ComponentStorage<Component>* get_or_create_storage() {
                std::type_index key = std::type_index(typeid(Component));
                auto iterator = _storages.find(key);
                if (iterator == _storages.end()) {
                    std::unique_ptr<ComponentStorage<Component>> storage = std::make_unique<ComponentStorage<Component>>();
                    ComponentStorage<Component>* ptr = storage.get();
                    _storages.emplace(key, std::move(storage));
                    return ptr;
                }
                return static_cast<ComponentStorage<Component>*>(iterator->second.get());
            }
        
            template<typename Component>
            ComponentStorage<Component>* get_storage() {
                std::type_index key = std::type_index(typeid(Component));
                auto iterator = _storages.find(key);
                if (iterator == _storages.end()) {
                    return nullptr;
                }
                return static_cast<ComponentStorage<Component>*>(iterator->second.get());
            }
        
            template<typename Component>
            const ComponentStorage<Component>* get_storage() const {
                std::type_index key = std::type_index(typeid(Component));
                auto iterator = _storages.find(key);
                if (iterator == _storages.end()) return nullptr;
                return static_cast<const ComponentStorage<Component>*>(iterator->second.get());
            }
        
            template<typename Function, typename... Components>
            void invoke_with_components(Function &&function, Entity entity) {
                auto tuple_of_refs = std::forward_as_tuple((*get_storage<Components>()->try_get(entity))...);
                std::apply([&](auto&... comps) {
                    std::invoke(function, entity, comps...);
                }, tuple_of_refs);
            }

        public:
            Registry() = default;
            ~Registry() = default;
        
            Entity create_entity() {
                Entity id = _nextId++;
                _alive.push_back(id);
                return id;
            }
        
            void destroy_entity(Entity entity) {
                for (auto& pair : _storages) {
                    pair.second->remove(entity);
                }
                auto iterator = std::find(_alive.begin(), _alive.end(), entity);
                if (iterator != _alive.end()) {
                    _alive.erase(iterator);
                }
            }
        
            bool valid(Entity entity) const {
                return std::find(_alive.begin(), _alive.end(), entity) != _alive.end();
            }
        
            template<typename Component, typename... Args>
            Component& emplace_component(Entity entity, Args&&... args) {
                if (!valid(entity)) {
                    ERROR("emplace_component on invalid entity");
                    throw std::runtime_error("emplace_component on invalid entity");
                }
                ComponentStorage<Component>* storage = get_or_create_storage<Component>();
                return storage->emplace(entity, std::forward<Args>(args)...);
            }
        
            template<typename Component>
            void remove_component(Entity entity) {
                ComponentStorage<Component>* storage = get_storage<Component>();
                if (storage) {
                    storage->remove(entity);
                }
            }
        
            template<typename Component>
            bool has_component(Entity entity) const {
                const ComponentStorage<Component>* storage = get_storage<Component>();
                return storage ? storage->contains(entity) : false;
            }
        
            template<typename Component>
            Component& get_component(Entity entity) {
                ComponentStorage<Component>* storage = get_storage<Component>();
                if (storage == nullptr) {
                    ERROR("get_component: storage doesn't exist");
                    throw std::runtime_error("get_component: storage doesn't exist");
                }
                Component* component = storage->try_get(entity);
                if (component == nullptr) {
                    ERROR("get_component: component missing for entity");
                    throw std::runtime_error("get_component: component missing for entity");
                }
                return *component;
            }
        
            template<typename Component>
            const Component& get_component(Entity entity) const {
                const ComponentStorage<Component>* storage = get_storage<Component>();
                if(storage == nullptr) {
                    ERROR("Failed retrieving storage, iterator does not exist.");
                    throw std::runtime_error("Failed retrieving storage, iterator does not exist.");
                }
                const Component* component = storage->try_get(entity);
                if (component == nullptr) {
                    ERROR("Failed retrieving components, entity does not exist.");
                    throw std::runtime_error("Failed retrieving components, entity does not exist.");
                }
                return *component;
            }
        
            template<typename Component>
            Component* try_get_component(Entity entity) {
                ComponentStorage<Component>* storage = get_storage<Component>();
                if (!storage) return nullptr;
                return storage->try_get(entity);
            }
        
            template<typename Component>
            const Component* try_get_component(Entity entity) const {
                const ComponentStorage<Component>* storage = get_storage<Component>();
                if (!storage) {
                    return nullptr;
                }
                return storage->try_get(entity);
            }
        
            template<typename... Components, typename Function>
            void each(Function &&function) {
                if constexpr (sizeof...(Components) == 0) {
                    for (Entity entity : _alive) {
                        std::invoke(function, entity);
                    }
                    return;
                }
        
                std::vector<const IComponentStorage*> storages = { get_storage<Components>()... };
                if (std::any_of(storages.begin(), storages.end(), [](const IComponentStorage* storage) { 
                    return storage == nullptr;
                })) {
                    return;
                }
        
                const IComponentStorage* smallest = storages.front();
                size_t minimumSize = smallest->entities().size();
                for (const IComponentStorage* storage : storages) {
                    size_t size = storage->entities().size();
                    if (size < minimumSize) {
                        minimumSize = size;
                        smallest = storage;
                    }
                }
        
                std::vector<Entity> candidateEntities = smallest->entities();
                for (Entity entity : candidateEntities) {
                    if (!(... && get_storage<Components>()->contains(entity))) {
                        continue;
                    }
                    invoke_with_components<Function, Components...>(std::forward<Function>(function), entity);
                }
            }
    };
}