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
        ~state() {
            for(auto [i, t] : m_types) delete t;
        };
        
        //T must not be ref or cv type
        template<typename T, std::enable_if_t<std::is_same_v<std::remove_reference_t<std::remove_cv_t<T>>, T>, bool> = true>
        type& register_type(std::string_view type_name) { 
            auto i = std::type_index(typeid(T));

            SGL_ASSERT(m_types.find(type_name) == m_types.end(), "check for state object already contains same type");
            SGL_ASSERT(m_type_names.find(i) == m_type_names.end(), "check for state object already contains type with same name");
            
            auto t = new type(sgl_type_identity<T>{}, type_name, this);

            m_types[t->m_type_name] = t;
            m_type_names[i] = t->m_type_name;
            
            return *t;
        }
        //T must not be ref or cv type
        template<typename T, std::enable_if_t<std::is_same_v<std::remove_reference_t<std::remove_cv_t<T>>, T>, bool> = true>
        type& get_type() const { 
            //if constexpr(std::is_fundamental_v<T>)//TODO replace with constexpr val?
            return *m_types.at(m_type_names.at(std::type_index(typeid(T)))); 
        }
        type& get_type(std::string_view type_name) const { 
            return *m_types.at(type_name); 
        }
        //T must not be ref or cv type
        template<typename T, std::enable_if_t<std::is_same_v<std::remove_reference_t<std::remove_cv_t<T>>, T>, bool> = true>
        void remove_type() { 
            auto i = std::type_index(typeid(T));
            m_types.erase(m_type_names.at(i));
            m_type_names.erase(i);
        }
        void remove_type(std::string_view type_name) { 
            m_type_names.erase(m_types.at(type_name)->m_type);
            m_types.erase(type_name);
        }
    protected:
        friend class type;

    private:
        std::unordered_map<std::type_index, std::string_view> m_type_names;
        std::unordered_map<std::string_view, type*> m_types;
    };
}//namespace SGL

#endif//SGL_STATE_HPP_INCLUDE_