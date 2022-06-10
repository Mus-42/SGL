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
        struct [[nodiscard]] value_creator_base {
            value_creator_base() : m_type(value_type::construct_value_type<T>()) {}
            explicit value_creator_base(std::shared_ptr<value_type> type) : m_type(type) {}

            std::shared_ptr<value_type> m_type;
            bool need_free_data = false;
            union {
                void* m_data;
                const void* m_const_data;
            };
        };

        template<typename T>
        struct value_creator : public value_creator_base<T> {
            template<typename... Args>
            [[nodiscard]] value_creator(Args... args) {
                this->m_data = new T(args...);//`this->` required by GCC (idk why)
                this->need_free_data = true;
            }
            template<typename... Args>
            [[nodiscard]] explicit value_creator(std::shared_ptr<value_type> type, Args... args) : value_creator_base<T>(type) {
                this->m_data = new T(args...);
                this->need_free_data = true;
            }
        };
        template<typename T>
        struct const_value_creator : public value_creator_base<const T> {
            template<typename... Args>
            [[nodiscard]] const_value_creator(Args... args) {
                this->m_const_data = new const T(args...);
                this->need_free_data = true;
            }
            template<typename... Args>
            [[nodiscard]] explicit const_value_creator(std::shared_ptr<value_type> type, Args... args) : value_creator_base<const T>(type) {
                this->m_const_data = new const T(args...);
                this->need_free_data = true;
            }
        };
        template<typename T>
        struct reference_creator : public value_creator_base<T&> {
            [[nodiscard]] reference_creator(T& v) {
                this->m_data = &v;
            }
        };
        template<typename T>
        struct const_reference_creator : public value_creator_base<const T&> {
            [[nodiscard]] const_reference_creator(const T& v) {
                this->m_const_data = &v;
            }
        };
        template<typename T>
        struct array_creator : public value_creator_base<arr<T>> {
            [[nodiscard]] array_creator(const std::vector<T>& v) {}

            template<size_t N>
            [[nodiscard]] array_creator(T (&v)[N]) {}

            //TODO add impl
        };
        //TODO add other value creators
        
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
        value() : m_data(nullptr), m_type(value_type::construct_value_type<void>()), need_free_data(true) {}
        value(value&& v) : m_type(std::move(v.m_type)), need_free_data(v.need_free_data) {
            
            //if(m_type && v.m_data) {
            //    if(!(is_reference() || is_pointer())) m_data = new char[m_type->size()];//TODO move to value_type? 
            //    m_type->move_construct(m_data, v.m_data);
            //}
            m_data = v.m_data;

            v.m_data = nullptr;
            v.need_free_data = false;
        }
        value(const value& v) : m_type(v.m_type), need_free_data(v.need_free_data) {
            
            if(m_type && v.m_data) {
                //TODO do something with array
                if(!(is_reference() || is_pointer())) m_data = new char[m_type->size()];//TODO move to value_type? 
                m_type->copy_construct(m_data, v.m_data);
            }
        }
        value& operator=(value&& v) {
            free_data();
            m_type = std::move(v.m_type);
            need_free_data = v.need_free_data;


            //if(m_type && v.m_data) {
            //    if(!(is_reference() || is_pointer())) m_data = new char[m_type->size()];//TODO move to value_type? 
            //    m_type->move_assign(m_data, v.m_data);
            //}
            m_data = v.m_data;

            v.m_data = nullptr;
            v.need_free_data = false;
            return *this;
        }
        value& operator=(const value& v) { 
            free_data();
            m_type = v.m_type;
            need_free_data = v.need_free_data;

            if(m_type && v.m_data) {
                if(!(is_reference() || is_pointer())) m_data = new char[m_type->size()];//TODO move to value_type? 
                m_type->copy_assign(m_data, v.m_data);
            }
            
            return *this;
        }

        template<typename T>
        value(details::value_creator_base<T>&& v) : m_data(v.m_data), m_type(std::move(v.m_type)), need_free_data(v.need_free_data) {
            
        }

        template<typename T>//create value without value_creator_...
        explicit value(details::sgl_type_identity<T>, typename details::sgl_type_identity<T>::type val) : m_type(value_type::construct_value_type<T>()) {
            if constexpr(std::is_reference_v<T>) {
                if constexpr(std::is_const_v<T>) m_const_data = &val;
                else m_data = &val;
            } else if constexpr(std::is_pointer_v<T>) {
                if constexpr(std::is_const_v<T>) m_const_data = val;
                else m_data = val;
            } 
            //else if constexpr(std::is_pointer_v<T>) {
            //    if constexpr(std::is_const_v<T>) m_const_data = val;
            //    else m_data = val;
            //}
        }

        
        
        template<typename T>
        [[nodiscard]] decltype(auto) get() {
            check_type(details::sgl_type_identity<T>{});
            return get(details::sgl_type_identity<T>{});
        }
        template<typename T>
        [[nodiscard]] decltype(auto) get() const {
            check_type(details::sgl_type_identity<T>{});
            return get(details::sgl_type_identity<T>{});
        }


        ~value() {
            free_data();
        }

        [[nodiscard]] bool is_array() const { return m_type->m_traits.is_array; }
        [[nodiscard]] bool is_pointer() const { return m_type->m_traits.is_pointer; }
        [[nodiscard]] bool is_reference() const { return m_type->m_traits.is_reference; }
        [[nodiscard]] bool is_const() const { return m_type->m_traits.is_const; }
        [[nodiscard]] bool is_void() const { return m_type->m_traits.is_void; }//TODO add other


    //protected:
        friend class state;
        friend class value_creator_base;
        
        template<typename T>//TODO remove it?
        static constexpr size_t arr_count(details::sgl_type_identity<T>) { return 0; }
        template<typename T>
        static constexpr size_t arr_count(details::sgl_type_identity<arr<T>>) { return vec_count(details::sgl_type_identity<T>{}) + 1; }

        //array
        template<typename T>
        decltype(auto) get(details::sgl_type_identity<arr<T>>) const {//const -> get by copy
            details::get_vector_from_arr_t<arr<T>> ret;
            get_vec_recursive(ret, m_data);
            return ret;//TODO check this is array & T is T array element type
        }
        //pointer   
        template<typename T>
        T* get(details::sgl_type_identity<T*>) const {//const -> pointer copy
            return static_cast<T*>(m_data);
        }
        template<typename T> decltype(auto) get(details::sgl_type_identity<T* const>) const { return get(details::sgl_type_identity<T*>{}); }
        template<typename T> decltype(auto) get(details::sgl_type_identity<T* volatile>) const { return get(details::sgl_type_identity<T*>{}); }
        template<typename T> decltype(auto) get(details::sgl_type_identity<T* const volatile>) const { return get(details::sgl_type_identity<T*>{}); }
        //reference
        template<typename T>
        T& get(details::sgl_type_identity<T&>) {//ref on value -> non const
            return *static_cast<T*>(m_data);
        }
        template<typename T>
        const T& get(details::sgl_type_identity<const T&>) const {//const ref
            return *static_cast<T*>(m_data);
        }
        template<typename T>
        T&& get(details::sgl_type_identity<T&&>) {//move -> not const
            return static_cast<T&&>(*static_cast<T*>(m_data));
        }
        //by value 
        template<typename T>
        T get(details::sgl_type_identity<T>) const {//copy this -> const
            return *static_cast<T*>(m_data);
        }

        template<typename T>
        void check_type(details::sgl_type_identity<T>) const {
            SGL_ASSERT(m_type->is_convertable_to<T>(), "this value not convertable to T");
        }


        template<typename T>
        void get_vec_recursive(std::vector<std::vector<T>>& ret, void* data) const {
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

        void free_data() {
            if(m_type && m_data) {
                m_type->free_ptr_of_t(m_data);
            }
        }
        union {
            void* m_data = nullptr;//if array - ptr to array_impl, if ref|ptr - their adress. if value - pointer to value
            const void* m_const_data;//ugly hack
        };
        std::shared_ptr<value_type> m_type;
        //TODO remove need_free_data? this checked in value_type::free_ptr_of_t()
        bool need_free_data = false;//example: references or pointers shod not freed. arrays or values must be freed
    };
}//namespace SGL

#endif//SGL_VALUE_HPP_INCLUDE_