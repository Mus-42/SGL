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
#include "function.hpp"

namespace SGL {
    class function;
    class state : public details::no_copy {
    public:
        state() {
            init();
        }

        //base type 

        template<details::req_base_type T>
        std::shared_ptr<base_type> register_type(const std::string& type_name) {
            if(!details::is_correct_identifier(type_name)) throw std::invalid_argument("incorrect type name: " + type_name);
            m_operator_list.add_default_operators_for_type<T>();
            return m_types_val[std::type_index(typeid(T))] = register_type(type_name, std::make_shared<base_type>(details::sgl_type_identity<T>{}));
        }
        std::shared_ptr<base_type> register_type(const std::string& type_name, std::shared_ptr<base_type> type_ptr) {
            if(!details::is_correct_identifier(type_name)) throw std::invalid_argument("incorrect type name: " + type_name);
            auto f = m_types.find(type_name);
            if(f != m_types.end()) throw std::runtime_error("type with same name already exists in this state");
            m_types[type_name] = type_ptr;
            return type_ptr;
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
        void add_function(const std::string& name, Ret(*f)(Args...)) {
            if(!details::is_correct_identifier(name)) throw std::invalid_argument("incorrect function name: " + name);
            m_functions[name].add_overload(f);
        }

        template<typename T>
        [[nodiscard]] std::shared_ptr<type> get_type() const {
            //TODO add type if it not exist?
            return type::construct_type<T>(m_types_val.at(std::type_index(typeid(details::make_base_type_t<T>))));
        }

        [[nodiscard]] function& get_function(const std::string& name) {
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

        template<typename T>
        void add_variable(const std::string& name, T v) {
            if(!details::is_correct_identifier(name)) throw std::invalid_argument("incorrect function name: " + name);
            m_variables[name] = value(details::sgl_type_identity<T>{}, v);
        }

        [[nodiscard]] value& get_variable(const std::string& name) {
            return m_variables.at(name);
        }
        [[nodiscard]] const value& get_variable(const std::string& name) const {
            return m_variables.at(name);
        }
        //TODO add remove variable?
    protected:
        friend class base_type;
        friend class type;
        friend class value;
        friend class evaluator;
        friend value details::eval_expr_rec_impl(const state& m_state, std::string_view base_str, std::string_view str, details::eval_impl_args args);

        std::unordered_map<std::string, std::shared_ptr<base_type>> m_types;
        std::unordered_map<std::type_index, std::shared_ptr<base_type>> m_types_val;

        std::unordered_map<std::string, function> m_functions;
        std::unordered_map<std::string, function> m_constructors;//and operators (syntax same: T(args))//TODO move to functions? (same syntax)
        
        std::unordered_map<std::string, value> m_variables;

        operator_list m_operator_list;
/*
        //TODO implement 
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
*/      
        template<typename From, typename To> 
        void add_typecast_between_impl(const std::string& type_name) {//Typecast for base types
            m_constructors[type_name].add_overload(std::function<To(const From&)>([](const From& v) -> To {
                return static_cast<To>(v);
            }));
        }
        template<typename T, typename... Args> 
        void add_constructor_impl(const std::string& type_name) {
            m_constructors[type_name].add_overload(std::function<T(Args...)>([](Args... args) -> T  {
                return T(args...);
            }));
        }

        template<typename... Types> 
        void add_binary_operator_permutations() {
            constexpr size_t types_count = sizeof...(Types);
            auto caller = [this]<size_t... i1>(std::index_sequence<i1...> s) constexpr {
                auto between_i_seq = [this]<size_t i2, size_t... j2>(details::sgl_value_identity<i2>, std::index_sequence<j2...>) constexpr {
                    auto between_i_j = [this]<size_t i, size_t j>(details::sgl_value_identity<i>, details::sgl_value_identity<j>) constexpr {
                        using A = std::remove_reference_t<std::tuple_element_t<i, std::tuple<Types...>>>;//strange usage of std::tuple
                        using B = std::remove_reference_t<std::tuple_element_t<j, std::tuple<Types...>>>;
                        if constexpr(i != j && !std::is_same_v<A, B>) 
                            m_operator_list.add_default_binary_operators_between_types<A, B>();       
                    };
                    (between_i_j(details::sgl_value_identity<i2>{}, details::sgl_value_identity<j2>{}), ...);
                };
                (between_i_seq(details::sgl_value_identity<i1>{}, s), ...);
            };
            caller(std::make_index_sequence<types_count>{});
        }

        
        //usage: add_constructors_impl<T, details::sgl_type_identity<Ags1...>, details::sgl_type_identity<Args2...>, ...>("T");
        template<typename To, typename... From>
        void add_constructors_impl(const std::string& type_name) {
            auto l = [&type_name, this]<typename... Args>(details::sgl_type_identity<Args...>){
                add_constructor_impl<To, Args...>(type_name);
            };
            (l(From{}), ...);
        }


        void init();
    };
}//namespace SGL

#endif//SGL_STATE_HPP_INCLUDE_