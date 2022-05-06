#pragma once
#ifndef SGL_STATE_HPP_INCLUDE_
#define SGL_STATE_HPP_INCLUDE_

#include "config.hpp"

#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <type_traits>
#include <stdexcept>

#include "type.hpp"

namespace SGL {
    class state {
    public:
        state() = default;
        //no copy
        state(const state&) = delete;
        state& operator=(const state&) = delete;
        //but move
        state(state&&) = default;
        state& operator=(state&&) = default;
        ~state() {
            for(auto [i, t] : m_types) delete t;
        };


        
        //T must not be ref or cv type
        template<typename T, typename... Members, std::enable_if_t<std::is_same_v<std::remove_reference_t<std::remove_cv<T>>, T>, bool> = true>
        type& register_type(std::initializer_list<std::string> members) { 
            if(members.size() != sizeof...(Members)) throw std::runtime_error("members variadic args and members must be same size");
            auto i = std::type_index(typeid(T));
            auto f = m_types.find(i);
            if(f != m_types.end()) throw std::runtime_error("state object already contains same type");
            auto v = (m_types[i] = new type);
            //TODO copy constuctors & destructors from T to v
            return *v;
        }
        //T must not be ref or cv type
        template<typename T, std::enable_if_t<std::is_same_v<std::remove_reference_t<std::remove_cv<T>>, T>, bool> = true>
        type& get_type() const { 
            if constexpr(std::is_fundamental_v<T>) return *m_types.at(std::type_index(typeid(T)));//TODO replace with constexpr val
            else return *m_types.at(std::type_index(typeid(T))); 
        }
        //T must not be ref or cv type
        template<typename T, std::enable_if_t<std::is_same_v<std::remove_reference_t<std::remove_cv<T>>, T>, bool> = true>
        void remove_type() { 
            m_types.erase(std::type_index(typeid(T)));
        }

    private:
        std::unordered_map<std::type_index, type*> m_types;
    };
};

#endif SGL_STATE_HPP_INCLUDE_