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

        //bool is_array() const { return m_type->traits.is_array; }
        //bool is_pointer() const { return m_type->traits.is_pointer; }
        //bool is_reference() const { return m_type->traits.is_reference; }

        //template<typename T>
        //bool is_same_type() const { 
        //}



    protected:
        friend class state;
        friend class value_type;

        value_type* m_type = nullptr;
        size_t array_size;
    };
}//namespace SGL

#endif//SGL_VALUE_HPP_INCLUDE_