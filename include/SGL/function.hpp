#pragma once
#ifndef SGL_FUNCTION_HPP_INCLUDE_
#define SGL_FUNCTION_HPP_INCLUDE_

#include "config.hpp"
#include "value.hpp"

#include <vector>
#include <functional>
#include <tuple>

namespace SGL {
    class value;
    class state;

    class function {
    protected:
        friend class state;
        friend class value;
        friend class operator_list;

        struct function_overload {
            function_overload() = default;
            function_overload(const function_overload&) = default;
            function_overload(function_overload&&) = default;
            function_overload&operator=(const function_overload&) = default;
            function_overload&operator=(function_overload&&) = default;
            
            struct m_impl_base {
                virtual value call(const std::vector<value>&) const = 0;
                virtual ~m_impl_base() = default;
            };

            template<typename Ret, typename... Args>
            function_overload(std::function<Ret(Args...)> func) : m_func(get_function_impl(details::sgl_function_identity<Ret(Args...)>{}, func, std::index_sequence_for<Args...>{})), args_types({type::construct_type<Args>()...}) {}
            template<typename Ret, typename... Args>
            function_overload(Ret(*func)(Args...)) : m_func(get_function_impl(details::sgl_function_identity<Ret(Args...)>{}, func, std::index_sequence_for<Args...>{})), args_types({type::construct_type<Args>()...}) {}

            template<typename Func, typename Ret, typename... Args, size_t... N>
            static constexpr decltype(auto) get_function_impl(details::sgl_function_identity<Ret(Args...)>, Func func, std::index_sequence<N...>) {
                struct func_impl : m_impl_base {
                    func_impl(Func func) : func(func) {}
                    virtual ~func_impl() = default;
                    virtual value call(const std::vector<value>& args) const override {
                        if(sizeof...(Args) != args.size()) throw std::runtime_error("sgl function_overload: invalid args count");
                        if constexpr(std::is_same_v<Ret, void>) {
                            func((args[N]).get<Args>() ...);
                            return value();            
                        }
                        else return value(val<Ret>(func((args[N]).get<Args>() ...)));//TODO replase val with ..?
                    }
                    Func func;
                };
                return new func_impl(func);
            }

            using m_func_ptr_t = value(*)(const std::vector<value>&);
            std::shared_ptr<m_impl_base> m_func;
            std::vector<std::shared_ptr<type>> args_types;
            
            int all_args_count = 0;
            bool all_types = false;
        protected:
            friend class function;

            friend class state;
            friend class value;
            friend class operator_list;

            struct all_types_t {};
            //constructors for built-in functions
            function_overload(m_func_ptr_t f, all_types_t, int all_args_count = -1) : m_func([f](){
                struct func_impl : m_impl_base {
                    func_impl(m_func_ptr_t f) : func(f) {}
                    virtual ~func_impl() = default;
                    virtual value call(const std::vector<value>& args) const override {
                        return func(args);
                    }
                    m_func_ptr_t func;
                };
                return new func_impl(f);
            }()), all_args_count(all_args_count), all_types(true) {}
            
            //template<typename Ret, typename... Args>
            //function_overload(std::function<Ret(Args...)> func, std::vector<std::shared_ptr<type>> args) : m_func(get_function_impl(func, std::index_sequence_for<Args...>{})), args_types(args) {
            //    if(sizeof...(Args) != args.size()) throw std::runtime_error("sgl function_overload: invalid args count");
            //}//TODO implement
        };
    public:
        [[nodiscard]] function() = default;
        [[nodiscard]] function(const function&) = default; 
        [[nodiscard]] function(function&&) = default; 
        function& operator=(const function&) = default; 
        function& operator=(function&&) = default; 

        [[nodiscard]] function(std::initializer_list<function_overload> func) : m_overloads(func) {}
        template<typename Ret, typename... Args>
        [[nodiscard]] function(std::function<Ret(Args...)> func) : m_overloads({{func}}) {}

        value call(const std::vector<value>& v) const {
            std::vector<std::pair<size_t, size_t>> indexes;
            for(size_t i = 0; i < m_overloads.size(); i++) 
                if(m_overloads[i].args_types.size() == v.size()) {
                    size_t delta = 0;
                    bool ok = true;
                    for(size_t j = 0; j < v.size(); j++) {
                        if(*m_overloads[i].args_types[j] == *v[j].m_type) continue;
                        if(v[j].m_type->is_convertable_to(*m_overloads[i].args_types[j])) { delta++; continue; }
                        ok = false;
                        break;
                    }
                    if(!ok) continue;
                    indexes.push_back({i, delta});
                } else if(m_overloads[i].all_types && (m_overloads[i].all_args_count == (int)v.size() || m_overloads[i].all_args_count == -1)) {
                    indexes.push_back({i, 0});
                }
            if(indexes.size() == 0) throw std::runtime_error("can't choose function overload");
            std::sort(indexes.begin(), indexes.end(), [](auto a, auto b){ return a.second < b.second; });
            if(indexes.size() != 1 && indexes[0].second == indexes[1].second) throw std::runtime_error("can't choose function overload: more than 1 candidate with same priority");
            return m_overloads[indexes.front().first].m_func->call(v);
        }
        
        template<typename Ret, typename... Args>
        function& add_overload(std::function<Ret(Args...)> func) {
            m_overloads.push_back({func});
            return *this;
        }
        function& merge(const function& v) {//TODO rename to merge_overloads
            size_t s = m_overloads.size();
            m_overloads.resize(s + v.m_overloads.size());
            std::copy(v.m_overloads.begin(), v.m_overloads.end(), m_overloads.begin() + s);
            return *this;
        }

        //TODO add std::function<...> get_cpp_func()?
    protected:
        function& add_overload(const function_overload& func) {
            m_overloads.push_back(func);
            return *this;
        }
        std::vector<function_overload> m_overloads;
    };

    //TODO add overloaded_function or something like this
}//namespace SGL

#endif//SGL_FUNCTION_HPP_INCLUDE_