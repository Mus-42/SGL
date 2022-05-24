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
    class state : public no_copy {
    public:
        state() = default;
        //TODO add impl with new type & value_type classes
        /*~state() {
            for(auto [i, t] : m_types) delete t;
        };
        
        template<typename T, std::enable_if_t<is_base_type<T>, bool> = true>
        type& register_type(std::string_view type_name) { 
            auto i = std::type_index(typeid(T));

            SGL_ASSERT(m_types.find(type_name) == m_types.end(), "check for state object already contains same type");
            SGL_ASSERT(m_type_names.find(i) == m_type_names.end(), "check for state object already contains type with same name");
            
            auto t = new type(sgl_type_identity<T>{}, type_name, this);

            m_types[t->m_type_name] = t;
            m_type_names[i] = t->m_type_name;
            
            return *t;
        }
        */

    protected:
        friend class type;

    private:
        //std::unordered_map<std::string_view, type*> m_types;//TODO replace with shared pointer

    };
}//namespace SGL

#endif//SGL_STATE_HPP_INCLUDE_