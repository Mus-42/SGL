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
        state() {
            //builtin types:

            //integer
            register_type<builtin_types::sgl_int8_t >("int8" );
            register_type<builtin_types::sgl_int16_t>("int16");
            register_type<builtin_types::sgl_int32_t>("int32");
            register_type<builtin_types::sgl_int64_t>("int64");

            register_type<builtin_types::sgl_uint8_t >("uint8" );
            register_type<builtin_types::sgl_uint16_t>("uint16");
            register_type<builtin_types::sgl_uint32_t>("uint32");
            register_type<builtin_types::sgl_uint64_t>("uint64");

            register_type<builtin_types::sgl_int_t>("int");
            register_type<builtin_types::sgl_uint_t>("uint");
            
            //floating point
            register_type<builtin_types::sgl_float32_t>("float32");
            register_type<builtin_types::sgl_float64_t>("float64");

            register_type<builtin_types::sgl_float_t>("float");
            register_type<builtin_types::sgl_double_t>("double");

            //other
            register_type<builtin_types::sgl_bool_t>("bool");
            register_type<builtin_types::sgl_char_t>("char");
            register_type<builtin_types::sgl_string_t>("string");

            //TODO register operators
        }

        //base type 
        template<typename T, std::enable_if_t<details::is_base_type<T>, bool> = true>
        std::shared_ptr<type> register_type(const std::string& type_name) {
            return m_types_val[std::type_index(typeid(T))] = register_type(type_name, std::make_shared<type>(details::sgl_type_identity<T>{}));
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

        //TODO add register_operator or something like this?

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
        std::unordered_map<std::type_index, std::shared_ptr<type>> m_types_val;
    };
}//namespace SGL

#endif//SGL_STATE_HPP_INCLUDE_