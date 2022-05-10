#pragma once
#ifndef SGL_FUNCTION_HPP_INCLUDE_
#define SGL_FUNCTION_HPP_INCLUDE_

#include "config.hpp"
#include "value.hpp"

#include <vector>
#include <functional>

namespace SGL {

    //TODO add overloaded_function or something like this

    //TODO convert Args to type*
    class value;
    class state;
    class function {
    public:
        template<typename Ret, typename... Args>
        function(std::function<Ret(Args...)> func) : m_func(get_function_impl(func, std::index_sequence_for<Args>{})) {
        }
        value call(std::initializer_list<std::reference_wrapper<value>> v) {
            return m_func(v);
        }
    protected:
        template<typename Ret, typename... Args, size_t... N>
        static constexpr decltype(auto) get_function_impl(std::function<Ret(Args...)> func, std::index_sequence<N...>) {
            return [func, s](std::initializer_list<std::reference_wrapper<value>> args) -> value {
                SGL_ASSERT(sizeof...(Args) == args.size(), "invalid args count");
                if constexpr(std::is_same_v<Ret, void>) {
                    func((std::data(args)[N]).get().get<Args>() ...);
                    return value();            
                }
                else return value(func((std::data(args)[N]).get().get<Args>() ...));
            };
        }

        std::function<value(std::initializer_list<std::reference_wrapper<value>>)> m_func;
    };
}//namespace SGL
 
#endif//SGL_FUNCTION_HPP_INCLUDE_