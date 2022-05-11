#pragma once
#ifndef SGL_VALUE_HPP_INCLUDE_
#define SGL_VALUE_HPP_INCLUDE_

#include "config.hpp"
#include "type.hpp"

namespace SGL {
    namespace details {
        struct array_impl {
            size_t m_size = 0;//elements count
            void* m_elements = nullptr;
        };
        //value_creator
        //const_value_creator
        //reference_creator
        //const_reference_creator
        template<typename T>
        struct array_creator {
            array_creator(std::vector<T> v) {}
            template<size_t N>
            array_creator(T v[N]) {}

            //TODO add impl
        };
        
    }//namespace details

    //TODO value construct: value(const_val(12)); 
    //make using for it
    //val
    //const_val
    //ref
    //const_ref
    //array
    //const_array
    //move_val
    //move_array

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
        
        template<typename T>
        static constexpr size_t vec_count(sgl_type_identity<T> v) { return 0; }
        template<typename T>
        static constexpr size_t vec_count(sgl_type_identity<std::vector<T>> v) { return vec_count(sgl_type_identity<T>{}) + 1; }

        //array
        template<typename T>
        decltype(auto) get(sgl_type_identity<std::vector<T>> v) const {//const -> get by copy
            //make it can return std::vector<T> or std::vector<std::vector<Y>> std::vector<std::vector<std::vector<Z>>> ...
            return std::vector<T>();//TODO check this is array & T is T array element type
        }
        template<typename T, size_t N> decltype(auto) get(sgl_type_identity<T[N]> v) const { return get(sgl_type_identity<std::vector<T>>{}); } // TODO check size?
        template<typename T> decltype(auto) get(sgl_type_identity<T[]> v) const { return get(sgl_type_identity<std::vector<T>>{}); } // TODO check size?
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

        
        void* m_data = nullptr;//if array - ptr to array_impl, if ref|ptr - their adress. if value - pointer to value
        value_type m_type;
    };
}//namespace SGL

#endif//SGL_VALUE_HPP_INCLUDE_