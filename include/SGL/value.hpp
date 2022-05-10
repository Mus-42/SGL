#pragma once
#ifndef SGL_VALUE_HPP_INCLUDE_
#define SGL_VALUE_HPP_INCLUDE_

#include "config.hpp"
#include "type.hpp"

namespace SGL {

    class value {
    public:
        value() {}
        value(value&& v) : m_type(std::move(v.m_type)) {
            //TODO call move | copy from value_type
        }
        value(const value& v): m_type(v.m_type) {
        }
        value& operator=(value&& v) {

        }
        value& operator=(const value& v) { 
        }


        template<typename T>
        explicit value(sgl_type_identity<T> v, typename sgl_type_identity<T>::type val, value_type&& t) : m_type(t) {
            
        }//TODO copy data?

        template<typename T>
        explicit value(value_type&& t) : m_type(t) {}//TODO copy data?

        
        
        template<typename T>
        decltype(auto) get() {
            return get(sgl_type_identity<T>{});
        }
        template<typename T>
        decltype(auto) get() const {
            return get(sgl_type_identity<T>{});
        }


        ~value() {
            if(m_data) delete m_data;
        }

        bool is_array() const { return m_type.m_traits.is_array; }
        bool is_pointer() const { return m_type.m_traits.is_pointer; }
        bool is_reference() const { return m_type.m_traits.is_reference; }

    protected:
        friend class state;
        friend class value_creator;
        
        //array
        template<typename T>
        std::vector<T> get(sgl_type_identity<T[]> v) const {//const -> get by copy
            return std::vector<T>();//TODO check this is array & T is T array element type
        }
        template<typename T, size_t N> decltype(auto) get(sgl_type_identity<T[N]> v) const { return get(sgl_type_identity<T[]>{}); } // TODO check size?
        //pointer   
        template<typename T>
        T* get(sgl_type_identity<T*> v) const {//const -> pointer copy
            return nullptr;//TODO return data?
        }
        template<typename T> decltype(auto) get(sgl_type_identity<T* const> v) const { return get(sgl_type_identity<T*>{}); }
        template<typename T> decltype(auto) get(sgl_type_identity<T* volatile> v) const { return get(sgl_type_identity<T*>{}); }
        template<typename T> decltype(auto) get(sgl_type_identity<T* const volatile> v) const { return get(sgl_type_identity<T*>{}); }
        //reference
        template<typename T>
        T& get(sgl_type_identity<T&> v) {//ref on value -> non const
            return *static_cast<T*>(nullptr);//TODO return data
        }
        template<typename T>
        const T& get(sgl_type_identity<const T&> v) const {//const ref
            return *static_cast<T*>(nullptr);//TODO return data
        }
        template<typename T>
        T&& get(sgl_type_identity<T&&> v) {//move -> not const
            return static_cast<T&&>(*static_cast<T*>(nullptr));//TODO return data
        }
        //by value 
        template<typename T>
        T get(sgl_type_identity<T> v) const {//copy this -> const
            return T();//TODO return data
        }

        
        void* m_data = nullptr;
        value_type m_type;
    };
}//namespace SGL

#endif//SGL_VALUE_HPP_INCLUDE_