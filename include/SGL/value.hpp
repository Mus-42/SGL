#pragma once
#ifndef SGL_VALUE_HPP_INCLUDE_
#define SGL_VALUE_HPP_INCLUDE_

#include "config.hpp"
#include "type.hpp"

namespace SGL {
    class value {
    public:
        value() {}
        ~value() {}
        //TODO: get<T>() -> try cast value to T. set<T> -> free old, assign new value
        //array, link, pointer & classes support
        //(call functions??)
    };
}

#endif//SGL_VALUE_HPP_INCLUDE_