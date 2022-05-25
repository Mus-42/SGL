#pragma once
#ifndef SGL_TYPE_HPP_INCLUDE_
#define SGL_TYPE_HPP_INCLUDE_

#include "config.hpp"
#include "utils.hpp"
//#include "function.hpp"

#include <cstdint>
#include <string>
#include <memory>//smart pointers
#include <unordered_map>

#include <type_traits>

namespace SGL {
    namespace details {
        class type_impl_base {
        public:
            virtual void default_construct(void* data) const {}
            virtual void copy_construct(void* data, const void* from) const {}
            virtual void move_construct(void* data, void* from) const {}

            virtual void copy_assign(void* data, const void* from) const {}
            virtual void move_assign(void* data, void* from) const {}

            //TODO add custom constructors? (pass it as SGL::function?)

            virtual void destruct(void* data) const {}

            virtual ~type_impl_base() {}
            struct {
                bool is_default_constructible = false;
                bool is_copy_constructible = false;
                bool is_move_constructible = false;

                bool is_copy_assignable = false;
                bool is_move_assignable = false;
            } traits;
            size_t size;
        };

        template<typename T>
        class type_impl : public type_impl_base {
        public:
            constexpr type_impl() {
                traits = {
                    std::is_default_constructible_v<T>,
                    std::is_copy_constructible_v<T>,
                    std::is_move_constructible_v<T>,

                    std::is_copy_assignable_v<T>,
                    std::is_move_assignable_v<T>
                };
                size = sizeof(T);
            }

            virtual void default_construct(void* data) const override { 
                if constexpr (std::is_default_constructible_v<T>) new (data) T(); 
            }
            virtual void copy_construct(void* data, const void* from) const override { 
                if constexpr (std::is_copy_constructible_v<T>) new (data) T(*static_cast<const T*>(from)); 
            }
            virtual void move_construct(void* data, void* from) const override { 
                if constexpr (std::is_move_constructible_v<T>) new (data) T(std::move(*static_cast<T*>(from))); 
            }

            virtual void copy_assign(void* data, const void* from) const override {
                if constexpr (std::is_copy_assignable_v<T>) *static_cast<T*>(data) = *static_cast<const T*>(from); 
            }
            virtual void move_assign(void* data, void* from) const override {
                if constexpr (std::is_move_assignable_v<T>) *static_cast<T*>(data) = std::move(*static_cast<T*>(from)); 
            }

            //TODO add custom constructors?

            virtual void destruct(void* data) const override {
                static_cast<T*>(data)->~T();
            }


            virtual ~type_impl() {}
        };
    }//namespace details

    class state;

    class type : public details::no_copy {
    public:
        template<typename T>
        [[nodiscard]] explicit type(details::sgl_type_identity<T> t) : m_impl(new details::type_impl<T>), m_type(typeid(T)) {
            //SGL_ASSERT(is_correct_identifier(type_name), "type name is incorrect");
        }
        ~type() = default;

        template<typename T> void default_construct(T& data) const { check_type<T>(); m_impl->default_construct(&data); }
        template<typename T> void copy_construct(T& data, const T& from) const { check_type<T>(); m_impl->copy_construct(&data, &from); }
        template<typename T> void move_construct(T& data, T&& from) const { check_type<T>(); m_impl->move_construct(&data, &from); }
        //TODO add custom constructors?
        template<typename T> void destruct(T& data) const { check_type<T>(); m_impl->destruct(&data); }

        
        template<typename T> void copy_assign(T& data, const T& from) const { check_type<T>(); m_impl->copy_assign(&data, &from); }
        template<typename T> void move_assign(T& data, T&& from) const { check_type<T>(); m_impl->move_assign(&data, &from); }
/*
        template<typename T, typename U> type& add_member(const std::string& member_name, U T::*member_ptr) {
            check_type<T>();
            const auto member_t = &m_state->get_type<U>();

            add_member(member_name, member_t, m_offsetof(member_ptr));

            return *this;
        }

        template<typename U, typename T> U& get_member(T& val, const std::string& member_name) const {
            check_type<T>();
            auto [member_t, offset] = m_members.at(member_name);
            member_t->check_type<U>();
            return *reinterpret_cast<U*>(reinterpret_cast<char*>(&val) + offset);
        } 
*/      
        //TODO fix members

        bool operator==(const type& v) const {
            return false;//TODO implement
        }
        bool operator!=(const type& v) const {
            return !(*this == v);
        }
    protected:
        friend class state;
        friend class value;
        friend class value_type;
        
        const std::unique_ptr<details::type_impl_base> m_impl;
        const type_info& m_type;//used in state and in check_type

        //std::unordered_map<std::string, std::pair<const type*, size_t>> m_members;//name, type, offset of member

        template<typename T>
        void check_type() const {
#if defined(SGL_OPTION_TYPE_CHECKS) && SGL_OPTION_TYPE_CHECKS
            SGL_ASSERT(m_type == typeid(T), "type specified in constructor call must me same with T");
#endif//SGL_OPTION_TYPE_CHECKS
        }

        //TODO not-template constructor. (for *.sgl deifned types). cannot cast value to C++ type. but can cast members?
/*
        void add_member(const std::string& member_name, const type* member_t, size_t offset) {
            SGL_ASSERT(m_members.find(member_name) == m_members.end(), "type contains member with same name");
            m_members[member_name] = { member_t,  offset };
        }
*/
        template<typename U, typename T> 
        static constexpr size_t m_offsetof(U T::*member_ptr) {
            return reinterpret_cast<size_t>(&(static_cast<T*>(nullptr)->*member_ptr));
        }
    };

   
    class value_type {
    public:
        value_type() : m_traits(details::sgl_type_identity<void>{}) {}

        value_type(const value_type&) = default;
        value_type(value_type&&) = default;
        value_type& operator=(const value_type&) = default;
        value_type& operator=(value_type&&) = default;

        template<typename T>
        bool is_same_with() const {//same with T
            return false;//TODO add impl
        } 
        template<typename T>
        bool is_convertable_to() const {//convertable to T
            return false;//TODO add impl
        }

        bool is_same_with(const value_type& t) const {//same with t
            if(m_traits != t.m_traits) return false;
            if(m_traits.is_final_v) return *m_base_type == *t.m_base_type;
            else return m_type->is_same_with(*t.m_type);
        }
        bool is_convertable_to(const value_type& t) const {//convertable to t
            return false;//TODO add impl
        }

        static value_type common_type(const value_type& a, const value_type& b) {//a & b convertable to common_type(a, b);
            return value_type();//TODO implement
        }
    //protected:
        friend class value;
        
        std::shared_ptr<value_type> m_type;
        std::shared_ptr<type> m_base_type;

        struct m_traits_t {
            m_traits_t() = default;
            m_traits_t(const m_traits_t&) = default;
            m_traits_t& operator=(const m_traits_t&) = default;
            ~m_traits_t() = default;

            template<typename T>
            constexpr explicit m_traits_t(details::sgl_type_identity<T> t) :
                is_const(std::is_const_v<T>), 
                is_pointer(std::is_pointer_v<T>), 
                is_reference(std::is_reference_v<T>),
                is_array(details::is_sgl_array_v<T>),
                is_void(std::is_same_v<T, void>),
                is_final_v(false)//set it manually
                {}
            
            bool is_const       : 1;
            bool is_pointer     : 1;
            bool is_reference   : 1;
            bool is_array       : 1;//array size stored in array_impl
            bool is_void        : 1;//unitililized value also void
            bool is_final_v     : 1;//value_type = (?const) base_type (?(*|&|&&))
            bool is_temp_v      : 1;//for language-temporary values. for example: a = 1 + 2; 3 - temp_v
        
            constexpr bool operator==(const m_traits_t& other) const {
                return is_const     == other.is_const
                    && is_pointer   == other.is_pointer
                    && is_reference == other.is_reference
                    && is_array     == other.is_array
                    && is_void      == other.is_void
                    && is_final_v   == other.is_final_v;//TODO add is_temp_v to compare?
            }
            bool operator!=(const m_traits_t& other) const { return !(*this == other); }
        } m_traits;

        //TODO move it to constuctor?
        template<typename T>
        static std::shared_ptr<value_type> construct_value_type() {//TODO get T
            static_assert(!std::is_array_v<T>);
            return construct_value_type_impl(details::sgl_type_identity<T>{});
        }
        //simple value
        template<typename T>
        static std::shared_ptr<value_type> construct_value_type_impl(details::sgl_type_identity<T> v) {
            auto ret = std::make_shared<value_type>();
            ret->m_traits = m_traits_t(v);
            ret->m_traits.is_final_v = true;
            ret->m_base_type = std::make_shared<type>(details::sgl_type_identity<T>{});
            return ret;
        }
        //reference
        template<typename T>
        static std::shared_ptr<value_type> construct_value_type_impl(details::sgl_type_identity<T&> v) {
            auto ret = std::make_shared<value_type>();
            ret->m_type = construct_value_type_impl(details::sgl_type_identity<T>{});
            ret->m_traits = m_traits_t(v);
            return ret;
        }
        //pointer
        template<typename T>
        static std::shared_ptr<value_type> construct_value_type_impl(details::sgl_type_identity<T*> v) {
            auto ret = std::make_shared<value_type>();
            ret->m_type = construct_value_type_impl(details::sgl_type_identity<T>{});
            ret->m_traits = m_traits_t(v);
            return ret;
        }
        template<typename T> static std::shared_ptr<value_type> construct_value_type_impl(details::sgl_type_identity<T*const> v) { 
            auto ret = std::make_shared<value_type>();
            ret->m_type = construct_value_type_impl(details::sgl_type_identity<T>{});
            ret->m_traits = m_traits_t(v);
            return ret;
        }
        
        //ignore volatile
        template<typename T> static std::shared_ptr<value_type> construct_value_type_impl(details::sgl_type_identity<T*volatile> v) { return construct_value_type_impl(details::sgl_type_identity<T*>{}); }
        template<typename T> static std::shared_ptr<value_type> construct_value_type_impl(details::sgl_type_identity<T*const volatile> v) { return construct_value_type_impl(details::sgl_type_identity<T*const>{}); }
        
        //array
        template<typename T>
        static std::shared_ptr<value_type> construct_value_type_impl(details::sgl_type_identity<arr<T>> v) {
            auto ret = std::make_shared<value_type>();
            ret->m_type = construct_value_type_impl(details::sgl_type_identity<T>{});
            ret->m_traits = m_traits_t(v);
            return ret;
        }

    };
} // namespace SGL

#endif// SGL_TYPE_HPP_INCLUDE_