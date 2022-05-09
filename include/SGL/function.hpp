#pragma once
#ifndef SGL_FUNCTION_HPP_INCLUDE_
#define SGL_FUNCTION_HPP_INCLUDE_

#include "config.hpp"
//#include "value.hpp"

#include <vector>
#include <functional>

namespace SGL {

    //TODO add overloaded_function or something like this

    //TODO convert Args to type*
    class value;
    class function {
    public:
        template<typename Ret, typename... Args>
        function(std::function<Ret(Args...)> func) {}
        
        template<typename Ret, typename... Args>
        function(Ret(*func)(Args...)) {}
    protected:
        //std::function<value(std::initializer_list<value*>)> m_func;
    };
}//namespace SGL

#endif//SGL_FUNCTION_HPP_INCLUDE_