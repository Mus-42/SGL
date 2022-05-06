#pragma once
#ifndef SGL_VALUE_HPP_INCLUDE_
#define SGL_VALUE_HPP_INCLUDE_

#include <vector>
#include <functional>

namespace SGL {

    //TODO add overloaded_function or something like this

    //TODO convert Args to type*

    class function {
    public:
        template<typename Ret, typename... Args>
        function(std::function<Ret(Args...)> func) {}
        
        template<typename Ret, typename... Args>
        function(Ret(*func)(Args...)) {}
    private:

    };
}

#endif//SGL_VALUE_HPP_INCLUDE_