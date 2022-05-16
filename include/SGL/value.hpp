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
            //TODO resizeble array_impl??
        };
        template<typename T>
        struct value_creator_base {
            value_type m_type;
            union {
                void* m_data;
                const void* m_const_data;
            };
        };

        template<typename T>
        struct value_creator : public value_creator_base<T> {
            value_creator(const T& v) {
                m_data = new T(v);
            }
        };
        template<typename T>
        struct const_value_creator : public value_creator_base<const T> {
            const_value_creator(const T& v) {
                m_const_data = new const T(v);
            }
        };
        template<typename T>
        struct reference_creator : public value_creator_base<T&> {
            reference_creator(T& v) {
                m_data = &v;
            }
        };
        template<typename T>
        struct const_reference_creator : public value_creator_base<const T&> {
            const_reference_creator(const T& v) {
                m_const_data = &v;
            }
        };
        template<typename T>
        struct array_creator : public value_creator_base<arr<T>> {
            array_creator(const std::vector<T>& v) {}
            template<size_t N>
            array_creator(T v[N]) {}

            //TODO add impl
        };
        
    }//namespace details

    //TODO value construct: value(const_val(12)); 
    //make using for it
    template<typename T> using val = details::value_creator<T>;
    template<typename T> using const_val = details::const_value_creator<T>;
    template<typename T> using ref = details::reference_creator<T>;
    template<typename T> using const_ref = details::const_reference_creator<T>;
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
        value(details::value_creator_base<T>&& v) : m_data(v.m_data), m_type(std::move(v.m_type)) {

        }

        template<typename T>
        explicit value(sgl_type_identity<T> v, typename sgl_type_identity<T>::type val, value_type&& t) : m_type(t) {
            
        }//TODO copy data?

        template<typename T>
        explicit value(value_type&& t) : m_type(t) {}//TODO copy data?

        
        
        template<typename T>
        decltype(auto) get() {
            //TODO nullptr check
            check_type(sgl_type_identity<T>{});
            return get(sgl_type_identity<T>{});
        }
        template<typename T>
        decltype(auto) get() const {
            check_type(sgl_type_identity<T>{});
            return get(sgl_type_identity<T>{});
        }


        ~value() {
            if(m_data) delete m_data;//TODO delete only if m_type is value
        }

        bool is_array() const { return m_type.m_traits.is_array; }
        bool is_pointer() const { return m_type.m_traits.is_pointer; }
        bool is_reference() const { return m_type.m_traits.is_reference; }

        

    //protected:
        friend class state;
        friend class value_creator_base;
        
        template<typename T>
        static constexpr size_t arr_count(sgl_type_identity<T> v) { return 0; }
        template<typename T>
        static constexpr size_t arr_count(sgl_type_identity<arr<T>> v) { return vec_count(sgl_type_identity<T>{}) + 1; }

        //array
        template<typename T>
        decltype(auto) get(sgl_type_identity<arr<T>> v) const {//const -> get by copy
            get_vector_from_arr_t<arr<T>> ret;
            get_vec_recursive(ret, m_data);
            return ret;//TODO check this is array & T is T array element type
        }
        //template<typename T, size_t N> decltype(auto) get(sgl_type_identity<T[N]> v) const { return get(sgl_type_identity<std::vector<T>>{}); } // TODO check size?
        //template<typename T> decltype(auto) get(sgl_type_identity<T[]> v) const { return get(sgl_type_identity<std::vector<T>>{}); } // TODO check size?
        
        //pointer   
        template<typename T>
        T* get(sgl_type_identity<T*> v) const {//const -> pointer copy
            return static_cast<T*>(m_data);
        }
        template<typename T> decltype(auto) get(sgl_type_identity<T* const> v) const { return get(sgl_type_identity<T*>{}); }
        template<typename T> decltype(auto) get(sgl_type_identity<T* volatile> v) const { return get(sgl_type_identity<T*>{}); }
        template<typename T> decltype(auto) get(sgl_type_identity<T* const volatile> v) const { return get(sgl_type_identity<T*>{}); }
        //reference
        template<typename T>
        T& get(sgl_type_identity<T&> v) {//ref on value -> non const
            return *static_cast<T*>(m_data);
        }
        template<typename T>
        const T& get(sgl_type_identity<const T&> v) const {//const ref
            return *static_cast<T*>(m_data);
        }
        template<typename T>
        T&& get(sgl_type_identity<T&&> v) {//move -> not const
            return static_cast<T&&>(*static_cast<T*>(m_data));
        }
        //by value 
        template<typename T>
        T get(sgl_type_identity<T> v) const {//copy this -> const
            return *static_cast<T*>(m_data);
        }

        template<typename T>
        void check_type(sgl_type_identity<T> v) const {
            //TODO check m_base_type is vector ... 
            //SGL_ASSERT(typeid(make_base_type_t<T>) == m_type.m_base_type->m_type, "invalid base type");

            //TODO compare value type
        }


        template<typename T>
        void get_vec_recursive(std::vector<std::vector<T>>& ret, void* data) const {
            //TODO check if m_base_type is vector<...>
            auto& d = *static_cast<details::array_impl*>(data);
            ret.resize(d.m_size);
            for(size_t i = 0; i < d.m_size; i++) get_vec_recursive(ret[i], static_cast<char*>(d.m_elements) + sizeof(details::array_impl) * i);
        }
        template<typename T>
        void get_vec_recursive(std::vector<T>& ret, void* data) const {
            auto& d = *static_cast<details::array_impl*>(data);
            auto els = static_cast<T*>(d.m_elements);
            ret = std::vector<T>(els, els+d.m_size);
        }

        
        void* m_data = nullptr;//if array - ptr to array_impl, if ref|ptr - their adress. if value - pointer to value
        value_type m_type;
    };
}//namespace SGL

#endif//SGL_VALUE_HPP_INCLUDE_