#pragma once
#ifndef SGL_FUNCTION_HPP_INCLUDE_
#define SGL_FUNCTION_HPP_INCLUDE_

#include "config.hpp"
#include "value.hpp"

#include <vector>
#include <functional>
#include <tuple>

namespace SGL {
    class value;
    class state;

    class function {
    protected:
        friend class type;
        friend class value;
        friend class state;

        struct function_overload {
            function_overload() = default;
            function_overload(const function_overload&) = default;
            function_overload(function_overload&&) = default;
            function_overload&operator=(const function_overload&) = default;
            function_overload&operator=(function_overload&&) = default;

            template<typename Ret, typename... Args>
            function_overload(std::function<Ret(Args...)> func) : m_func(get_function_impl(func, std::index_sequence_for<Args...>{})), args_types({value_type::construct_value_type<Args>()...}) {}

            template<typename Ret, typename... Args, size_t... N>
            static constexpr decltype(auto) get_function_impl(std::function<Ret(Args...)> func, std::index_sequence<N...>) {
                return [func](std::initializer_list<std::reference_wrapper<value>> args) -> value {
                    SGL_ASSERT(sizeof...(Args) == args.size(), "invalid args count");
                    if constexpr(std::is_same_v<Ret, void>) {
                        func((std::data(args)[N]).get().get<Args>() ...);
                        return value();            
                    }
                    else return value(val<Ret>(func((std::data(args)[N]).get().get<Args>() ...)));//TODO replase val with ..?
                };
            }

            std::vector<std::shared_ptr<value_type>> args_types;
            std::function<value(std::initializer_list<std::reference_wrapper<value>>)> m_func;
            
            bool all_types = false;
            int all_args_count = 0;
        protected:
            friend class function;
            friend class state;
            friend class value;

            struct all_types_t {};
            //constructors for built-in functions
            function_overload(std::function<value(std::initializer_list<std::reference_wrapper<value>>)> f, std::vector<std::shared_ptr<value_type>> args) : m_func(f), args_types(args) {}
            function_overload(std::function<value(std::initializer_list<std::reference_wrapper<value>>)> f, all_types_t all, int all_args_count = -1) : m_func(f), all_types(true), all_args_count(all_args_count) {}
            
            template<typename Ret, typename... Args>
            function_overload(std::function<Ret(Args...)> func, std::vector<std::shared_ptr<value_type>> args) : m_func(get_function_impl(func, std::index_sequence_for<Args...>{})), args_types(args) {
                SGL_ASSERT(args.size() == sizeof...(Args), "invalid args count");
            }
        };
    public:
        [[nodiscard]] function() = default;
        [[nodiscard]] function(const function&) = default; 
        [[nodiscard]] function(function&&) = default; 
        function& operator=(const function&) = default; 
        function& operator=(function&&) = default; 

        [[nodiscard]] function(std::initializer_list<function_overload> func) : m_overloads(func) {}
        template<typename Ret, typename... Args>
        [[nodiscard]] function(std::function<Ret(Args...)> func) : m_overloads({{func}}) {}

        value call(std::initializer_list<std::reference_wrapper<value>> v) const {
            std::vector<std::pair<size_t, size_t>> indexes;
            for(size_t i = 0; i < m_overloads.size(); i++) 
                if(m_overloads[i].args_types.size() == v.size()) {
                    size_t delta = 0;
                    bool ok = true;
                    for(size_t j = 0; j < v.size(); j++) {
                        if(*m_overloads[i].args_types[j] == *v.begin()[j].get().m_type) continue;
                        //TODO if convertable { delta++; continue; }
                        ok = false;
                        break;
                    }
                    if(!ok) continue;
                    indexes.push_back({i, delta});
                } else if(m_overloads[i].all_types && (m_overloads[i].all_args_count == v.size() || m_overloads[i].all_args_count == -1)) {
                    indexes.push_back({i, 0});
                }
            SGL_ASSERT(indexes.size() > 0, "cant choose function overload");
            //TODO sort by delta
            return m_overloads[indexes.front().first].m_func(v);
        }
        
        template<typename Ret, typename... Args>
        function& add_overload(std::function<Ret(Args...)> func) {
            m_overloads.push_back({func});
            return *this;
        }
        function& merge(const function& v) {
            size_t s = m_overloads.size();
            m_overloads.resize(s + v.m_overloads.size());
            std::copy(v.m_overloads.begin(), v.m_overloads.end(), m_overloads.begin() + s);
            return *this;
        }
    protected:
        std::vector<function_overload> m_overloads;
    };

    //TODO add overloaded_function or something like this
}//namespace SGL

#endif//SGL_FUNCTION_HPP_INCLUDE_