#pragma once
#ifndef SGL_UTILS_HPP_INCLUDE_
#define SGL_UTILS_HPP_INCLUDE_

#include "config.hpp"
#include <cctype>//std::isalpha, std::isalnum ...
#include <algorithm>
#include <vector>

namespace SGL {
    namespace details {   
        template<typename T>
        struct array_value {
            using type = T;
            static_assert(!std::is_reference_v<T>, "arr<T&>: cannot create array of references. did you mean pointers instead of references?");
        };
    }//namespace details
    
    template<typename T> using arr = details::array_value<T>;

    //TODO move utils to details namespace?
    
    class no_copy {
    public:
        no_copy() = default;
        ~no_copy() = default;

        no_copy(const no_copy&) = delete;
        no_copy& operator=(const no_copy&) = delete;

        no_copy(no_copy&&) = default;
        no_copy& operator=(no_copy&&) = default;
    };

    template<typename T> struct sgl_type_identity {
        using type = T;
    };

    constexpr bool is_correct_identifier(std::string_view s) {
        if(!s.size() || !std::isalpha(s[0]) && s[0] != '_') return false;
        return std::all_of(s.begin(), s.end(), [](unsigned char ch) -> bool { return std::isalnum(ch) || ch == '_'; });
    }

    template<typename T>
    struct make_base_type {
        using type = std::remove_cv_t<T>;
    };
    //pointer
    template<typename T>
    struct make_base_type<T*> {
        using type = typename make_base_type<std::remove_pointer_t<T>>::type;
    };
    template<typename T>
    struct make_base_type<T* const> {
        using type = typename make_base_type<std::remove_pointer_t<T>>::type;
    };
    template<typename T>
    struct make_base_type<T* volatile> {
        using type = typename make_base_type<std::remove_pointer_t<T>>::type;
    };
    template<typename T>
    struct make_base_type<T* const volatile> {
        using type = typename make_base_type<std::remove_pointer_t<T>>::type;
    };
    //reference
    template<typename T>
    struct make_base_type<T&> {
        using type = typename make_base_type<std::remove_reference_t<T>>::type;
    };
    template<typename T>
    struct make_base_type<T&&> {
        using type = typename make_base_type<std::remove_reference_t<T>>::type;
    };
    //array
    /*
    template<typename T>
    struct make_base_type<T[]> {
        using type = typename make_base_type<std::remove_all_extents_t<T>>::type;
    };
    template<typename T, size_t N>
    struct make_base_type<T[N]> {
        using type = typename make_base_type<std::remove_all_extents_t<T>>::type;
    };*/
    template<typename T>
    struct make_base_type<arr<T>> {
        using type = typename make_base_type<T>::type;
    };

    template<typename T>
    using make_base_type_t = typename make_base_type<T>::type;

    template<typename T>
    constexpr bool is_base_type = std::is_same_v<T, make_base_type_t<T>>;


    
    template<typename T, typename F>
    constexpr decltype(auto) for_each_type_decorator(sgl_type_identity<T> t, F func) {
        //no decorator
        if constexpr(!std::is_same_v<T, std::remove_cv_t<T>>) func(t);
    }

    //pointer
    template<typename T, typename F>
    constexpr decltype(auto) for_each_type_decorator(sgl_type_identity<T*> t, F func) {
        func(t);
        for_each_type_decorator(sgl_type_identity<T>{}, func);
    }
    template<typename T, typename F>
    constexpr decltype(auto) for_each_type_decorator(sgl_type_identity<T*const> t, F func) {
        func(t);
        for_each_type_decorator(sgl_type_identity<T>{}, func);
    }
    template<typename T, typename F>
    constexpr decltype(auto) for_each_type_decorator(sgl_type_identity<T*volatile> t, F func) {
        func(t);
        for_each_type_decorator(sgl_type_identity<T>{}, func);
    }
    template<typename T, typename F>
    constexpr decltype(auto) for_each_type_decorator(sgl_type_identity<T*const volatile> t, F func) {
        func(t);
        for_each_type_decorator(sgl_type_identity<T>{}, func);
    }
    //reference
    template<typename T, typename F>
    constexpr decltype(auto) for_each_type_decorator(sgl_type_identity<T&> t, F func) {
        func(t);
        for_each_type_decorator(sgl_type_identity<T>{}, func);
    }
    template<typename T, typename F>
    constexpr decltype(auto) for_each_type_decorator(sgl_type_identity<T&&> t, F func) {
        func(t);
        for_each_type_decorator(sgl_type_identity<T>{}, func);
    }
    //array
    /*
    template<typename T, size_t N, typename F>
    constexpr decltype(auto) for_each_type_decorator(sgl_type_identity<T[N]> t, F func) {
        func(t);
        for_each_type_decorator(sgl_type_identity<T>{}, func);      
    }
    template<typename T, typename F>
    constexpr decltype(auto) for_each_type_decorator(sgl_type_identity<T[]> t, F func) {
        func(t);
        for_each_type_decorator(sgl_type_identity<T>{}, func);
    }
    */
    template<typename T, typename F>
    constexpr decltype(auto) for_each_type_decorator(sgl_type_identity<arr<T>> t, F func) {
        func(t);
        for_each_type_decorator(sgl_type_identity<T>{}, func);
    }

    template<typename T>
    struct get_vector_from_arr {
        using type = T;
        using base_type = T;
    };
    template<typename T>
    struct get_vector_from_arr<arr<T>> {
        using type = std::vector<typename get_vector_from_arr<T>::type>;
        using base_type = typename get_vector_from_arr<T>::base_type;
    };
    template<typename T> using get_vector_from_arr_t = typename get_vector_from_arr<T>::type;
    template<typename T> using get_vector_from_arr_b = typename get_vector_from_arr<T>::base_type;


    template<typename T>
    struct is_sgl_array {
        constexpr static bool value = false;
    };
    template<typename T>
    struct is_sgl_array<arr<T>> {
        constexpr static bool value = true;
    };
    template<typename T>
    constexpr bool is_sgl_array_v = is_sgl_array<T>::value;
}//namespace SGL

#endif//SGL_UTILS_HPP_INCLUDE_