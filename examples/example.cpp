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

    //state().get_evaluator()//invalid 

    try {
        auto v = value(val<std::string>("some text"));
        std::cout << "get<std::string>() " << v.get<std::string>() << std::endl;
        std::cout << "get<const std::string&>() " << v.get<const std::string&>() << std::endl;
        //std::cout << "to_string()" << v.to_string() << std::endl;
        std::cout << "get<int>() " << v.get<int>() << std::endl;
    } 
    catch(const std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    auto st = state();
    auto ev = st.get_evaluator();

    //ev.evaluate(tokenizer("int a = -100u * 10;"));
    //ev.evaluate(tokenizer("0xFFFF + 0b01101101u8"));
    //ev.evaluate(tokenizer("1.12 + 48u32"));
    //ev.evaluate(tokenizer("1.12e2"));
    //ev.evaluate(tokenizer("1.12e-2"));
    //ev.evaluate(tokenizer("1.12e+2f"));
    //ev.evaluate(tokenizer(R"("qq" + "\tall\n")"));
    //ev.evaluate(tokenizer("auto v = {1, 4.26, \"mimsus\"};"));

    auto v = st.register_type<int>("my_int");
    SGL_ASSERT(v->m_type == typeid(int), "type check");

    st.add_typecast_between_types<int, float, double>();
    {
        auto arg1 = value(val<int>(12));
        auto arg2 = value(const_val<double>(12.42));

        //auto result = st.m_operator_list.call_operator(operator_type::op_typecast, {arg1});
        //how it must coose correct operator (In wich type it must cast value)?
        //TODO make typecast not-operator function? (as constructor) 

        //std::cout << result.get<float>() << std::endl;
    }
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
//*/
/*
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
    //*/
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
    //*/
}