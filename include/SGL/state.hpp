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
#include "operator_list.hpp"

namespace SGL {
    class function;
    class state : public details::no_copy {
    public:
        state() {
            init();
        }

        //base type 

        template<typename T, std::enable_if_t<details::is_base_type<T>, bool> = true>
        std::shared_ptr<type> register_type(const std::string& type_name) {
            if(!details::is_correct_identifier(type_name)) throw std::invalid_argument("incorrect type name: " + type_name);
            m_operator_list.add_default_operators_for_type<T>();
            return m_types_val[std::type_index(typeid(T))] = register_type(type_name, std::make_shared<type>(details::sgl_type_identity<T>{}));
        }
        std::shared_ptr<type> register_type(const std::string& type_name, std::shared_ptr<type> type_ptr) {
            if(!details::is_correct_identifier(type_name)) throw std::invalid_argument("incorrect type name: " + type_name);
            auto f = m_types.find(type_name);
            if(f != m_types.end()) throw std::runtime_error("type with same name already exists in this state");
            m_types[type_name] = type_ptr;
            return type_ptr;
        }
        std::shared_ptr<type> find_type(const std::string& type_name) const {
            return m_types.at(type_name);
        }

        void add_function(const std::string& name, const function& f) {
            if(!details::is_correct_identifier(name)) throw std::invalid_argument("incorrect function name: " + name);
            m_functions[name].merge(f);
        }
        template<typename Ret, typename... Args>
        void add_function(const std::string& name, std::function<Ret(Args...)> f) {
            if(!details::is_correct_identifier(name)) throw std::invalid_argument("incorrect function name: " + name);
            m_functions[name].add_overload(f);
        }

        template<typename Ret, typename... Args>
        void add_function_overload(const std::string& name, std::function<Ret(Args...)> f) {
            if(!details::is_correct_identifier(name)) throw std::invalid_argument("incorrect function name: " + name);
            m_functions[name].add_overload(f);
        }

        [[nodiscard]] std::shared_ptr<type> get_base_type(const std::string& name) const {
            return m_types.at(name);
        }
        template<typename T, std::enable_if_t<details::is_base_type<T>, bool> = true>
        [[nodiscard]] std::shared_ptr<type> get_base_type() const {
            return m_types_val.at(std::type_index(typeid(T)));
        }
        
        template<typename T>
        [[nodiscard]] std::shared_ptr<value_type> get_type() const {
            return value_type::construct_value_type<T>(m_types_val.at(std::type_index(typeid(details::make_base_type_t<T>))));
        }

        function& get_function(const std::string& name) {
            return m_functions.at(name);
        }
        [[nodiscard]] const function& get_function(const std::string& name) const {
            return m_functions.at(name);
        }

        //TODO add register_operator or something like this?

        [[nodiscard]] evaluator get_evaluator() const & {
            return evaluator(*this);
        }
        evaluator get_evaluator() const && = delete;//not alloved for temp state (ref invalid afrer object destruction)

        
    //protected:
        friend class type;
        friend class value_type;
        friend class value;
        friend class evaluator;

        template<typename... Types> 
        void add_typecast_between_types() {
            (add_typecast_between_types_impl(details::sgl_type_identity<Types>{}, details::sgl_type_identity<Types...>{}), ...);
        }
        template<typename From, typename... To> 
        void add_typecast_between_types_impl(details::sgl_type_identity<From>, details::sgl_type_identity<To...>) {
            (add_typecast_between_impl(details::sgl_type_identity<From>{}, details::sgl_type_identity<To>{}), ...);
        }
        
        template<typename T> 
        void add_typecast_between_impl(details::sgl_type_identity<T>, details::sgl_type_identity<T>) {}//discard same type
        
        template<typename From, typename To> 
        void add_typecast_between_impl(details::sgl_type_identity<From>, details::sgl_type_identity<To>) {
            //TODO change signature to To(const From&) ?
            //m_operator_list.add_operator<operator_type::op_typecast>(std::function<To(const From&)>([](const From& v)->To{ 
            //    std::cout << "typecats <" << get_type_name<From>() << "> to <" << get_type_name<To>() << '>' << std::endl;
            //    return static_cast<To>(v);
            //}));
        }
        std::unordered_map<std::string, std::shared_ptr<type>> m_types;
        std::unordered_map<std::type_index, std::shared_ptr<type>> m_types_val;

        std::unordered_map<std::string, function> m_functions;

        operator_list m_operator_list;

        //TODO add registred constructors?

        void init() {   
            //builtin types:
            register_type<void>("void");
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

            //TODO type for type (result of typeof)?

            //builtin functions:
            add_function("addressof", {{{std::function<value(std::initializer_list<std::reference_wrapper<value>>)>([this](std::initializer_list<std::reference_wrapper<value>> v)->value{
                if(v.size() != 1) throw std::runtime_error("addressof args count != 1");
                auto& q = v.begin()->get();
                if(q.is_const()) return { const_val<void*>(get_type<void*>(), q.m_data) };
                else return { const_val<void*>(get_type<void*>(), q.m_data) };
            }), function::function_overload::all_types_t{}, 1} }});
            add_function("sizeof", {{{std::function<value(std::initializer_list<std::reference_wrapper<value>>)>([this](std::initializer_list<std::reference_wrapper<value>> v)->value{
                if(v.size() != 1) throw std::runtime_error("sizeof args count != 1");
                auto& q = v.begin()->get();
                return { const_val<builtin_types::sgl_uint64_t>(get_type<builtin_types::sgl_uint64_t>(), q.m_type->size()) };
            }), function::function_overload::all_types_t{}, 1} }});

            //TODO register operators
        }
    };
}//namespace SGL

#endif//SGL_STATE_HPP_INCLUDE_