#include <SGL/SGL.hpp>
#include <iostream>

int main() {
    //TODO add example

    //some test code
    const SGL::type t_str(SGL::sgl_type_identity<std::string>{});

    char buf[sizeof(std::string)];
    auto& str = *reinterpret_cast<std::string*>(buf);
    t_str.default_construct(str);

    str += "cstdiofan";
    std::cout << str << std::endl;

    std::string other = "mimsus";

    t_str.move_construct(str, std::move(other)); 
    
    std::cout << str << std::endl;
    std::cout << other.size() << std::endl;
}