#pragma once
#ifndef SGL_TYPE_HPP_INCLUDE_
#define SGL_TYPE_HPP_INCLUDE_

#include "config.hpp"
#include "utils.hpp"

#include <cstdint>
#include <string>
#include <memory>//smart pointers
#include <unordered_map>
#include <sstream>//type::to_string()

#include <type_traits>
#include <string_view>

namespace SGL {
    namespace builtin_types {
        //integer
        using sgl_int8_t  = std::int8_t; 
        using sgl_int16_t = std::int16_t; 
        using sgl_int32_t = std::int32_t; 
        using sgl_int64_t = std::int64_t; 

        using sgl_uint8_t  = std::uint8_t; 
        using sgl_uint16_t = std::uint16_t; 
        using sgl_uint32_t = std::uint32_t; 
        using sgl_uint64_t = std::uint64_t; 

        using sgl_int_t = sgl_int32_t;  
        using sgl_uint_t = sgl_uint32_t;//changing type alias here not change it in state::init

        //floating point
        using sgl_float32_t = float;
        using sgl_float64_t = double;

        using sgl_float_t = sgl_float32_t;
        using sgl_double_t = sgl_float64_t;

        static_assert(sizeof(sgl_float32_t) == 32/8 && sizeof(sgl_float64_t) == 64/8);

        //boolean
        using sgl_bool_t = bool;
        //character
        using sgl_char_t = char;//using char8_t?
        //string
        using sgl_string_t = std::basic_string<sgl_char_t>;

        static_assert(!std::is_same_v<sgl_char_t, sgl_int8_t> && !std::is_same_v<sgl_char_t, sgl_uint8_t>, "can't overload to_string for chars");
        //TODO add unicode strings?
    }


    namespace details {
        class type_impl_base {
        public:
            constexpr type_impl_base() : size(0) {}
            virtual void default_construct([[maybe_unused]] void* data) const = 0;
            virtual void copy_construct([[maybe_unused]] void* data, [[maybe_unused]] const void* from) const = 0;
            virtual void move_construct([[maybe_unused]] void* data, [[maybe_unused]] void* from) const = 0;
            virtual void copy_assign([[maybe_unused]] void* data, [[maybe_unused]] const void* from) const = 0;
            virtual void move_assign([[maybe_unused]] void* data, [[maybe_unused]] void* from) const = 0;

            //TODO add custom constructors? (pass it as SGL::function?)

            //TODO add create_ptr_of_t? (and array version?)
            virtual void free_ptr_of_t([[maybe_unused]] void* data) const = 0;
            virtual std::string to_string([[maybe_unused]] const void* data) const = 0;
            //TODO add free_array_ptr_of_t? 

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
            static_assert(details::is_base_type<T>);
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

            virtual void free_ptr_of_t(void* data) const override {
                delete static_cast<T*>(data);
            }

            virtual std::string to_string(const void* data) const override {
                const T& v = *static_cast<const T*>(data);
                if constexpr(std::is_floating_point_v<T>) {
                    if(std::isfinite(v)) [[likely]] return std::to_string(v);
                    else [[unlikely]] {
                        if(std::isnan(v)) return "nan";
                        if(std::isinf(v)) return "inf";
                        return "[floating-point value]";
                    }
                } else if constexpr(std::is_same_v<builtin_types::sgl_char_t, T>) {
                    if(static_cast<unsigned char>(v) >= 128) [[unlikely]] return "[non-ascii char]";
                    switch (v) {
                    case '\\': return R"('\\')"; 
                    case '\'': return R"('\'')";
                    case '"' : return R"('"')";
                    case '\0': return R"('\0')";
                    case '\b': return R"('\b')";
                    case '\f': return R"('\f')";
                    case '\n': return R"('\n')";
                    case '\r': return R"('\r')"; 
                    case '\t': return R"('\t')";
                    default: break;
                    }
                    if(std::isprint(static_cast<unsigned char>(v))) [[likely]] {
                        char buf[4] = "'.'";
                        buf[1] = v;
                        return buf;
                    }
                    return "[non-print char]";
                } else if constexpr(std::is_same_v<builtin_types::sgl_string_t, T>) {
                    //TODO cast string to escape (same as char)
                    return v;
                } else if constexpr(requires(const T& v) {
                    { std::to_string(v) } -> std::convertible_to<std::string>;
                }) {
                    return std::to_string(v);//cast char as integer type
                } else if constexpr(requires(const T& v) {
                    { v.to_string() } -> std::convertible_to<std::string>;
                }) {
                    return v.to_string();
                } else if constexpr(requires(const T& v, std::ostream out) {
                    out << v;
                }) {//TODO how it must cast floating point numbers to string? (fixed precision?)
                    std::ostringstream s;
                    s << v;
                    return s.str();
                } else return std::string("[value of type ") + std::string(get_type_name<T>()) + std::string("]"); 
            }

            //TODO add custom constructors?

            virtual ~type_impl() {}
        };
        
        template<>
        class type_impl<void> : public type_impl_base {
        public:
            constexpr type_impl() {}

            virtual void default_construct(void*) const override {}
            virtual void copy_construct(void*, const void*) const override {}
            virtual void move_construct(void*, void*) const override {}
            virtual void copy_assign(void*, const void*) const override {}
            virtual void move_assign(void*, void*) const override {}
            
            virtual void free_ptr_of_t(void* data) const override {
                delete static_cast<char*>(data);
            }
            
            virtual std::string to_string(const void*) const override {
                return "[void]";    
            }

            virtual ~type_impl() {}
        };
    }//namespace details

    class state;

    class base_type : public details::no_copy {
    public:
        template<typename T>
        [[nodiscard]] explicit base_type(details::sgl_type_identity<T>) : m_impl(new details::type_impl<T>), m_type(typeid(T)) {
            m_type_name = get_type_name<T>();
        }
        ~base_type() = default;

        size_t size() const { return m_impl->size; }

        void default_construct(void* data) const { m_impl->default_construct(data); }
        void copy_construct(void* data, const void* from) const { m_impl->copy_construct(data, from); }
        void move_construct(void* data, void* from) const { m_impl->move_construct(data, from); }
        void copy_assign(void* data, const void* from) const { m_impl->copy_assign(data, from); }
        void move_assign(void* data, void* from) const { m_impl->move_assign(data, from); }
        void free_ptr_of_t(void* data) const { m_impl->free_ptr_of_t(data); }

        std::string to_string(const void* data) const { return m_impl->to_string(data); }
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

        bool operator==(const base_type& v) const {
            return m_type == v.m_type;
        }
        bool operator!=(const base_type& v) const {
            return !(*this == v);
        }
    //protected:
        friend class state;
        friend class value;
        friend class type;
        
        const std::unique_ptr<details::type_impl_base> m_impl;
        const std::type_info& m_type;//used in state and in check_type
        std::string_view m_type_name;

        //std::unordered_map<std::string, std::pair<const type*, size_t>> m_members;//name, type, offset of member
/*
        template<typename T>
        void check_type() const {//TODO remove it
#if defined(SGL_OPTION_TYPE_CHECKS) && SGL_OPTION_TYPE_CHECKS
            SGL_ASSERT(m_impl && m_type == typeid(T), "type specified in constructor call must me same with T");
#endif//SGL_OPTION_TYPE_CHECKS
        }
*/

        //TODO not-template constructor. (for *.sgl deifned types). cannot cast value to C++ type. but can cast members?
/*
        void add_member(const std::string& member_name, const type* member_t, size_t offset) {
            SGL_ASSERT(m_members.find(member_name) == m_members.end(), "type contains member with same name");
            m_members[member_name] = { member_t,  offset };
        }
*/
        template<typename U, typename T> 
        static constexpr size_t m_offsetof(U T::*member_ptr) {
            return reinterpret_cast<size_t>(static_cast<const char*>(&(static_cast<T*>(nullptr)->*member_ptr)));
        }
    };
    inline std::strong_ordering operator<=>(const base_type& a, const base_type& b) {
        auto at = std::type_index(a.m_type);
        auto bt = std::type_index(b.m_type);
        //clang 13 not support <=> operator for type_index
        if(at < bt) return std::strong_ordering::less;
        if(at > bt) return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }
   
    class type {
    public:
        type() : m_traits(details::sgl_type_identity<void>{}) {}

        type(const type&) = default;
        type(type&&) = default;
        type& operator=(const type&) = default;
        type& operator=(type&&) = default;

        template<typename T>
        bool is_same_with() const {//same with T
            if(m_traits != m_traits_t(details::sgl_type_identity<T>{})) return false;
            if(m_traits.is_final_v) return m_base_type->m_type == typeid(T);
            else return is_same_with(*construct_type<T>());
        } 
        template<typename T>
        bool is_convertable_to() const {//convertable to T
            return is_convertable_to(*construct_type<T>());//TODO replace construct type with something else?
        }

        bool is_same_with(const type& t) const {//same with t
            if(m_traits != t.m_traits) return false;
            if(m_traits.is_final_v) return *m_base_type == *t.m_base_type;
            else return m_type->is_same_with(*t.m_type);
        }

        bool is_convertable_to(const type& t) const {//convertable to t
            //TODO add pointers cast? search typecast operator in state 
            if(m_traits.is_pointer   != t.m_traits.is_pointer || 
               m_traits.is_array     != t.m_traits.is_array) return false;//in SGL array can't be casted to pointer
            if(m_traits.is_pointer) return (t.m_type->m_traits.is_const || !m_traits.is_const) && m_type->is_same_with(*t.m_type);
            if(m_traits.is_reference) {
                if(t.m_traits.is_reference) return (t.m_type->m_traits.is_const || !m_traits.is_const) && m_type->is_same_with(*t.m_type);//ref
                return m_type->is_convertable_to(t);//value
            }
            if(m_traits.is_final_v) {
                if(t.m_traits.is_reference) return t.m_type->m_traits.is_final_v && *m_base_type == *t.m_type->m_base_type && (t.m_type->m_traits.is_const || !m_traits.is_const);
                return *m_base_type == *t.m_base_type;//no const check -> const T can be casted to T
            }
            //array
            return false;//TODO add impl
        }

        //TODO replace `const type&` with `std::shared_ptr<type>`?
        static type common_type(const type& a, const type& b) {//a & b convertable to common_type(a, b);
            if(a.is_same_with(b)) return a;
            return type();//TODO implement
        }

        //TODO add is_array() is_pointer() ...?

        size_t size() const {
            if(m_traits.is_pointer || m_traits.is_reference) return sizeof(void*);
            else if(m_traits.is_final_v) return m_base_type->size();
            else {
                //TODO do something with array?
                return 0;
            }
        }

        void default_construct(void*& data) const {
            //if ref|pointer - nullptr
            //if value - construct using m_type
            //if array ... what i should do with size?
            if(m_traits.is_pointer || m_traits.is_reference) data = nullptr;
            else if(m_traits.is_final_v) m_base_type->default_construct(data);
            else {
                //TODO array
            }
        }
        void copy_construct(void*& data, void* from) const { 
            //copy ref|value|array
            if(m_traits.is_pointer || m_traits.is_reference) data = from;
            else if(m_traits.is_final_v) m_base_type->copy_construct(data, from);
            else {
                //TODO array
            }
        }
        void move_construct(void*& data, void* from) const {
            //move ref|value|array
            if(m_traits.is_pointer || m_traits.is_reference) data = from;
            else if(m_traits.is_final_v) m_base_type->move_construct(data, from);
            else {
                //TODO array
            }
        }
        void copy_assign(void*& data, void* from) const {
            //move ref|value|array
            if(m_traits.is_pointer || m_traits.is_reference) data = from;
            else if(m_traits.is_final_v) m_base_type->copy_assign(data, from);
            else {
                //TODO array
            }
        }
        void move_assign(void*& data, void* from) const {
            //move ref|value|array
            if(m_traits.is_pointer || m_traits.is_reference) data = from;
            else if(m_traits.is_final_v) m_base_type->move_assign(data, from);
            else {
                //TODO array
            }
        }
        void free_ptr_of_t(void*& data) const {
            if(!data) return;
            if(m_traits.is_final_v) m_base_type->free_ptr_of_t(data);
            //TODO free array
            data = nullptr;
        }

        std::string to_string(const void* data) const {
            if(m_traits.is_final_v) return m_base_type->to_string(data);
            else if(m_traits.is_reference && m_type->m_traits.is_final_v) return m_type->m_base_type->to_string(data);
            else [[unlikely]] {
                if(m_traits.is_pointer) [[likely]] return (std::stringstream() << data).str();
                return {};
            }//TODO add value of type ... here?
        }

        type& add_const() {//const int* -> const int* const
            if(m_traits.is_reference) m_type->add_const();
            else m_traits.is_const = true;
            return *this;
        }
        type& remove_const() {//const int* const -> const int*
            if(m_traits.is_reference) m_type->remove_const();
            else m_traits.is_const = false;
            return *this;
        }

        type& add_reference() {//int -> int&
            if(!m_traits.is_reference) {
                auto cur = std::make_shared<type>(std::move(*this));
                type ret;
                ret.m_type = std::move(cur);

                ret.m_traits.is_const     = false;
                ret.m_traits.is_pointer   = false;
                ret.m_traits.is_reference = true;
                ret.m_traits.is_array     = false;
                ret.m_traits.is_void      = false;
                ret.m_traits.is_final_v   = false;

                *this = std::move(ret);
            }
            return *this;
        }

        type& remove_reference() {//int& -> int
            if(m_traits.is_reference) *this = std::move(*m_type);
            return *this;
        }


    //protected:
        friend class value;
        friend class evaluator;
        friend class function;
        friend class operator_list;
        friend class value_creator_base;
        friend inline std::strong_ordering operator<=>(const type& a, const type& b);
        
        std::shared_ptr<type> m_type;
        std::shared_ptr<base_type> m_base_type;

        struct m_traits_t {
            m_traits_t() = default;
            m_traits_t(const m_traits_t&) = default;
            m_traits_t& operator=(const m_traits_t&) = default;
            ~m_traits_t() = default;

            template<typename T>
            constexpr explicit m_traits_t(details::sgl_type_identity<T>) :
                is_const(std::is_const_v<T>), 
                is_pointer(std::is_pointer_v<T>), 
                is_reference(std::is_reference_v<T>),
                is_array(details::is_sgl_array_v<T>),
                is_void(std::is_same_v<T, void>),
                is_final_v(false) //set it manually
                {}
            
            bool is_const     : 1;
            bool is_pointer   : 1;
            bool is_reference : 1;
            bool is_array     : 1;//array size stored in array_impl
            bool is_void      : 1;//unitililized value also void
            bool is_final_v   : 1;//type = (?const) base_type (?(*|&|&&))
        
            constexpr bool operator==(const m_traits_t& other) const {
                return is_const     == other.is_const
                    && is_pointer   == other.is_pointer
                    && is_reference == other.is_reference
                    && is_array     == other.is_array
                    && is_void      == other.is_void
                    && is_final_v   == other.is_final_v;
            }
            constexpr bool operator!=(const m_traits_t& other) const { return !(*this == other); }
            constexpr std::strong_ordering operator<=>(const m_traits_t& other) const = default;
        } m_traits;

        //TODO move it to constuctor?
        template<typename T>
        static std::shared_ptr<type> construct_type(const std::shared_ptr<base_type>& base_type_v = std::make_shared<base_type>(details::sgl_type_identity<details::make_base_type_t<T>>{})) {//TODO get T
            static_assert(!std::is_array_v<T>);
            return construct_type_impl(details::sgl_type_identity<T>{}, base_type_v);
        }
        //simple value
        template<typename T>
        static std::shared_ptr<type> construct_type_impl(details::sgl_type_identity<T> v, const std::shared_ptr<base_type>& base_type_v) {
            auto ret = std::make_shared<type>();
            ret->m_traits = m_traits_t(v);
            ret->m_traits.is_final_v = true;
            ret->m_base_type = base_type_v;
            return ret;
        }
        //reference
        template<typename T>
        static std::shared_ptr<type> construct_type_impl(details::sgl_type_identity<T&> v, const std::shared_ptr<base_type>& base_type_v) {
            auto ret = std::make_shared<type>();
            ret->m_type = construct_type_impl(details::sgl_type_identity<T>{}, base_type_v);
            ret->m_traits = m_traits_t(v);
            return ret;
        }
        //template<typename T>
        //static std::shared_ptr<type> construct_type_impl(details::sgl_type_identity<const T&> v, const std::shared_ptr<type>& base_type) {
        //    auto ret = std::make_shared<type>();
        //    ret->m_type = construct_type_impl(details::sgl_type_identity<T>{}, base_type);
        //    ret->m_traits = m_traits_t(v);
        //    return ret;
        //}
        //pointer
        template<typename T>
        static std::shared_ptr<type> construct_type_impl(details::sgl_type_identity<T*> v, const std::shared_ptr<base_type>& base_type_v) {
            auto ret = std::make_shared<type>();
            ret->m_type = construct_type_impl(details::sgl_type_identity<T>{}, base_type_v);
            ret->m_traits = m_traits_t(v);
            return ret;
        }
        template<typename T> static std::shared_ptr<type> construct_type_impl(details::sgl_type_identity<T*const> v, const std::shared_ptr<base_type>& base_type_v) { 
            auto ret = std::make_shared<type>();
            ret->m_type = construct_type_impl(details::sgl_type_identity<T>{}, base_type_v);
            ret->m_traits = m_traits_t(v);
            return ret;
        }
        
        //ignore volatile
        template<typename T> static std::shared_ptr<type> construct_type_impl(details::sgl_type_identity<T*volatile>, const std::shared_ptr<base_type>& base_type_v) { 
            return construct_type_impl(details::sgl_type_identity<T*>{}, base_type_v); 
        }
        template<typename T> static std::shared_ptr<type> construct_type_impl(details::sgl_type_identity<T*const volatile>, const std::shared_ptr<base_type>& base_type_v) { 
            return construct_type_impl(details::sgl_type_identity<T*const>{}, base_type_v); 
        }
        
        //array
        template<typename T>
        static std::shared_ptr<type> construct_type_impl(details::sgl_type_identity<arr<T>> v, const std::shared_ptr<base_type>& base_type_v) {
            auto ret = std::make_shared<type>();
            ret->m_type = construct_type_impl(details::sgl_type_identity<T>{}, base_type_v);
            ret->m_traits = m_traits_t(v);
            return ret;
        }

        
        std::string type_to_str() const {
            std::string ret;
            type_to_str_impl(ret, this);
            return ret;   
        }
        void type_to_str_impl(std::string& ret, const SGL::type* v) const {
            if(!v->m_traits.is_final_v) {
                if(v->m_traits.is_array) ret += "arr<";
                type_to_str_impl(ret, v->m_type.get());
                if(v->m_traits.is_array) ret += ">";

                if(v->m_traits.is_pointer) ret += "*";
                if(v->m_traits.is_reference) ret += "&";
                if(v->m_traits.is_const) ret += " const";
            }
            else {
                if(v->m_traits.is_const) ret += "const ";
                ret += v->m_base_type->m_type_name;
            }
        }
    };

    inline bool operator==(const type& a, const type& b) {
        return a.is_same_with(b);
    }
    inline bool operator!=(const type& a, const type& b) {
        return !(a==b);
    }
    inline std::strong_ordering operator<=>(const type& a, const type& b) {
        if(a.m_traits == b.m_traits) {
            if(a.m_traits.is_final_v) return *a.m_base_type <=> *b.m_base_type;
            else return *a.m_type <=> *b.m_type;
        }
        else return a.m_traits <=> b.m_traits;
    }

} // namespace SGL

#endif// SGL_TYPE_HPP_INCLUDE_