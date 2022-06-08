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
#include "evaluator.hpp"

namespace SGL {
    class state : public details::no_copy {
    public:
        state() = default;

        //base type 
        template<typename T, std::enable_if_t<details::is_base_type<T>, bool> = true>
        std::shared_ptr<type> register_type(const std::string& type_name) {
            return register_type(type_name, std::make_shared<type>(details::sgl_type_identity<T>{}));
        }
        std::shared_ptr<type> register_type(const std::string& type_name, std::shared_ptr<type> type_ptr) {
            auto f = m_types.find(type_name);
            SGL_ASSERT(f == m_types.end(), "type with same name already exists in this state")
            m_types[type_name] = type_ptr;
            return type_ptr;
        }
        std::shared_ptr<type> find_type(const std::string& type_name) const {
            return m_types.at(type_name);
        }


        [[nodiscard]] evaluator get_evaluator() const & {
            return evaluator(*this);
        }
        evaluator get_evaluator() const && = delete;//not alloved for temp state (ref invalid afrer object destruction)

    protected:
        friend class type;
        friend class value_type;
        friend class value;
        friend class evaluator;

    private:
        std::unordered_map<std::string, std::shared_ptr<type>> m_types;
    };
}//namespace SGL

#endif//SGL_STATE_HPP_INCLUDE_