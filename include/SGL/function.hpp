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
    public:
        [[nodiscard]] function() = default;
        [[nodiscard]] function(const function&) = default; 
        [[nodiscard]] function(function&&) = default; 
        function& operator=(const function&) = default; 
        function& operator=(function&&) = default; 

        template<typename Ret, typename... Args>
        [[nodiscard]] function(std::function<Ret(Args...)> func) : m_overloads({{func}}) {}

        value call(std::initializer_list<std::reference_wrapper<value>> v) {
            std::vector<std::pair<size_t, size_t>> indexes;
            for(size_t i = 0; i < m_overloads.size(); i++) if(m_overloads[i].args_types.size() == v.size()) {
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
            }
            SGL_ASSERT(indexes.size() > 0, "cant choose function overload");
            return m_overloads[indexes.front().first].m_func(v);
        }

        
        template<typename Ret, typename... Args>
        function& add_overload(std::function<Ret(Args...)> func) {
            m_overloads.push_back({func});
            return *this;
        }
    protected:

        struct function_overload {
            template<typename Ret, typename... Args>
            function_overload(std::function<Ret(Args...)> func) : m_func(get_function_impl(func, std::index_sequence_for<Args...>{})), args_types({value_type::construct_value_type<Args>()...}) {

            }

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
        };
        
        std::vector<function_overload> m_overloads;
    };

    //TODO add overloaded_function or something like this
}//namespace SGL
 
#endif//SGL_FUNCTION_HPP_INCLUDE_