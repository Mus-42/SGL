#pragma once
#ifndef SGL_VALUE_HPP_INCLUDE_
#define SGL_VALUE_HPP_INCLUDE_

#include "config.hpp"
#include "type.hpp"

namespace SGL {
    class value {
    public:
        value() {}
        value(value&& v) : m_type(std::move(v.m_type)), array_size(v.array_size) {        
            if(!m_type.m_decorators.empty() && (
                m_type.m_decorators.back() == value_type_decorator::d_pointer ||   
                m_type.m_decorators.back() == value_type_decorator::d_array   ||
                m_type.m_decorators.back() == value_type_decorator::d_reference//TODO add other
            )) {
                m_data = v.m_data;
                v.m_data = nullptr;
            }
        }
        value(const value& v): m_type(v.m_type), array_size(v.array_size) {
            if(!m_type.m_decorators.empty() && (
                m_type.m_decorators.back() == value_type_decorator::d_pointer ||   
                m_type.m_decorators.back() == value_type_decorator::d_array   ||
                m_type.m_decorators.back() == value_type_decorator::d_reference//TODO add other
            )) {
                m_data = v.m_data;
            }
        }//TODO copy data?
        value& operator=(value&& v) {
            m_type = std::move(v.m_type);
            array_size = v.array_size;
            if(!m_type.m_decorators.empty() && (
                m_type.m_decorators.back() == value_type_decorator::d_pointer ||   
                m_type.m_decorators.back() == value_type_decorator::d_array   ||
                m_type.m_decorators.back() == value_type_decorator::d_reference//TODO add other
            )) {
                m_data = v.m_data;
                v.m_data = nullptr;
            }
        }
        value& operator=(const value& v) { 
            m_type = v.m_type;
            array_size = v.array_size;
            if(!m_type.m_decorators.empty() && (
                m_type.m_decorators.back() == value_type_decorator::d_pointer ||   
                m_type.m_decorators.back() == value_type_decorator::d_array   ||
                m_type.m_decorators.back() == value_type_decorator::d_reference//TODO add other
            )) {
                m_data = v.m_data;
            }
        }


        template<typename T>
        explicit value(sgl_type_identity<T> v, typename sgl_type_identity<T>::type val, value_type&& t) : m_type(t) {
            if constexpr(std::is_array_v<T>) m_data = static_cast<void*>(val);
            else if constexpr(std::is_reference_v<T>) m_data = static_cast<void*>(&val);
            else if constexpr(std::is_pointer_v<T>) m_data = static_cast<void*>(val);
        }//TODO copy data?
        template<typename T>
        explicit value(value_type&& t) : m_type(t) {}//TODO copy data?

        template<typename T>
        T get() {
            //TODO check is T correct
            //TODO cast to T?
            if constexpr(std::is_array_v<T>) return static_cast<T>(m_data);
            else if constexpr(std::is_reference_v<T> ) {
                auto& v = *static_cast<std::remove_reference_t<T>*>(m_data);
                return v;
            }
            else if constexpr(std::is_pointer_v<T>) return static_cast<T>(m_data);
            else return *static_cast<T*>(m_data);//TODO fixit
        }
        template<typename T>
        const T get() const {
            if constexpr(std::is_array_v<T>) return static_cast<std::add_const_t<T>>(m_data);
            else if constexpr(std::is_reference_v<T> ) return *static_cast<std::remove_reference_t<T>*>(m_data);
            else if constexpr(std::is_pointer_v<T>) return static_cast<std::add_const_t<T>>(m_data);
            else return *static_cast<T*>(m_data);//TODO fixit
        }

        template<typename T>
        T&& move() {
            return static_cast<T&&>(T());//TODO get from data
        }


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


        value_type m_type;
        size_t array_size;

        void* m_data;
    };
}//namespace SGL

#endif//SGL_VALUE_HPP_INCLUDE_