#include <SGL/SGL.hpp>
#include <iostream>

int main() {
    using namespace SGL;

    auto st = state{};
    auto ev = st.get_evaluator();

    std::string expr_str;
    while(std::cout << ">>> ", std::getline(std::cin, expr_str), expr_str != "exit") try {
        std::cout << ev.evaluate_expression(expr_str).to_string() << std::endl;
    } catch(std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
}