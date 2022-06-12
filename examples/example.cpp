#include <SGL/SGL.hpp>
#include <SGL/function.hpp>

#include <iostream>

int main() {

    //TODO add example

    /*
        now value.get<T> is unsafe - it not check type cast correctness 
    */

    //some test code
    using namespace SGL;

    struct no_sum {};
    struct has_sum {has_sum operator+(const has_sum&) { return {}; }};
    struct has_plus {has_sum operator+() { return {}; }};
    struct has_subscript {int operator[](size_t) { return 10; }};

    std::cout << std::boolalpha << details::OperatorsExistCheck::op_sum<no_sum> << ' ' << details::OperatorsExistCheck::op_sum<has_sum> << std::endl;
    std::cout << std::boolalpha << details::OperatorsExistCheck::op_unary_plus<no_sum> << ' ' << details::OperatorsExistCheck::op_unary_plus<has_plus> << std::endl;
    std::cout << std::boolalpha << details::OperatorsExistCheck::op_subscript<no_sum> << ' ' << details::OperatorsExistCheck::op_subscript<has_subscript> << std::endl;
    
    static_assert(details::OperatorsExistCheck::op_subscript<has_subscript>);
    static_assert(details::OperatorsExistCheck::op_subscript<std::array<int, 10>>);
    static_assert(!details::OperatorsExistCheck::op_subscript<has_sum>);

    //state().get_evaluator()//invalid 

    auto st = state();
    auto ev = st.get_evaluator();

    ev.evaluate(tokenizer("int a = -100u * 10;"));
    ev.evaluate(tokenizer("0xFFFF + 0b01101101u8"));
    ev.evaluate(tokenizer("1.12 + 48u32"));
    ev.evaluate(tokenizer("1.12e2"));
    ev.evaluate(tokenizer("1.12e-2"));
    ev.evaluate(tokenizer("1.12e+2f"));
    ev.evaluate(tokenizer(R"("qq" + "\tall\n")"));

    auto v = st.register_type<int>("my_int");
    SGL_ASSERT(v->m_type == typeid(int), "type check");
/*
    auto val_t = SGL::value_type::construct_value_type<arr<const int>* const>();

    std::function<void(std::shared_ptr<SGL::value_type>)> type_print;
    type_print = [&type_print](std::shared_ptr<SGL::value_type> v){
        if(!v->m_traits.is_final_v) {
            if(v->m_traits.is_array) std::cout << "arr<";
            type_print(v->m_type);
            if(v->m_traits.is_array) std::cout << ">";

            if(v->m_traits.is_pointer) std::cout << "*";
            if(v->m_traits.is_reference) std::cout << "&";
            if(v->m_traits.is_const) std::cout << " const";
        }
        else {
            if(v->m_traits.is_const) std::cout << "const ";
            std::cout << 'T';
        }
    };

    type_print(val_t);
*/
    std::cout << std::endl;
    auto f = function({
        std::function<int(const int&)>([](const int& v){
            std::cout << "i: " << v << std::endl;
            return v;
        }), 
        std::function<void(double)>([](double v){ 
            std::cout << "f: " << v << std::endl;
        })
    });
    auto arg1 = value(val<int>(12));
    auto arg2 = value(const_val<double>(12.42));
    f.call({arg1});
    f.call({arg2});
 
    std::cout << "arg1* " << st.get_function("addressof").call({arg1}).get<void*>() << std::endl;
    std::cout << "arg2* " << st.get_function("addressof").call({arg2}).get<void*>() << std::endl;

    std::cout << "arg1 size " << st.get_function("sizeof").call({arg1}).get<uint64_t>() << std::endl;
    std::cout << "arg2 size " << st.get_function("sizeof").call({arg2}).get<uint64_t>() << std::endl;

    struct base {
        base() { std::cout << "base constructed" << std::endl; }
        base& operator=(const base&) { std::cout << "base copied" << std::endl; return *this; }
        virtual void say() const { std::cout << "im base" << std::endl; };
        virtual ~base() { std::cout << "base destructed" << std::endl; }
    };
    struct derived : base {
        derived() { std::cout << "derived constructed" << std::endl; }
        derived& operator=(const derived&) { std::cout << "derived copied" << std::endl; return *this; }
        virtual void say() const override { std::cout << "im derived" << std::endl; };
        virtual ~derived() { std::cout << "derived destructed" << std::endl; }
    };
    std::cout << "CPP:" << std::endl;
    {
        derived d1;
        derived d2;
        d2 = d1;
        d2.say();
        auto d3 = d1;
        const base& b = d2;
        b.say();
    }
    std::cout << "SGL:" << std::endl;
    {
        auto d1 = value(val<derived>{});
        auto d2 = value(val<derived>{});
        d2 = d1;
        d2.get<const derived&>().say();
        auto d3 = d1;
        auto b = value(const_ref<base>{d2.get<const derived&>()});
        b.get<const base&>().say();
    }
}