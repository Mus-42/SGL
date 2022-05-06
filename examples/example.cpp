#include <SGL/SGL.hpp>
#include <iostream>

int main() {
    const SGL::type t_str(SGL::sgl_type_identity<std::string>{});

    char buf[sizeof(std::string)];
    t_str.m_impl->default_construct(buf);
    auto& str = *reinterpret_cast<std::string*>(buf);

    str += "cstdiofan";
    std::cout << str << std::endl;

    std::string other = "mimsus";

    t_str.m_impl->move_construct(buf, &other); 
    
    std::cout << str << std::endl;
    std::cout << other.size() << std::endl;
}