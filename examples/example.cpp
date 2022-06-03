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

    auto ev = evaluator();
    ev.evaluate(tokenizer("int a = -100u * 10;"));
    ev.evaluate(tokenizer("0xFFFF + 0b001101101"));
    ev.evaluate(tokenizer("1.12 + 48ull"));
    ev.evaluate(tokenizer("1.12e2"));
    ev.evaluate(tokenizer("1.12e-2"));
    ev.evaluate(tokenizer("1.12e+2f"));
    ev.evaluate(tokenizer(R"("qq" + "\tall\n")"));

    auto val_t = SGL::value_type::construct_value_type<arr<const int&>* const>();

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
}