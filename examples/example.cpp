#include <SGL/SGL.hpp>
#include <iostream>

int main() {
    //TODO add example

    //some test code

    //const SGL::type t_str(SGL::sgl_type_identity<std::string>{}, "string");

    SGL::type t_int32(SGL::sgl_type_identity<int32_t>{}, "int32", nullptr);

    SGL::state s;
    auto& t_str = s.register_type<std::string>("string");   
    auto& t_vector_int = s.register_type<std::vector<int>>("vector_int");   

    char buf[sizeof(std::string)];
    auto& str = *reinterpret_cast<std::string*>(buf);
    t_str.default_construct(str);

    str += "cstdiofan";
    std::cout << str << std::endl;

    std::string other = "mimsus";

    t_str.move_construct(str, std::move(other)); 
    
    std::cout << str << std::endl;
    std::cout << other.size() << std::endl;

    struct struct_with_member {
        std::string s;
    };  

    auto& t_struct_with_member = s.register_type<struct_with_member>("struct_with_member");
    t_struct_with_member.add_member("s", &struct_with_member::s);

    char arr[sizeof(struct_with_member)];

    auto& struct_with_member_val = *reinterpret_cast<struct_with_member*>(arr);
    t_struct_with_member.default_construct(struct_with_member_val);    

    struct_with_member_val.s = "SimplifiedGenLang";

    std::cout << t_struct_with_member.get_member<std::string>(struct_with_member_val, "s") << std::endl;

    SGL::tokenizer tk("int a = /* comment */ b;//comment\nint c = 0;");
    

    SGL::value_type(SGL::sgl_type_identity<int * const[100]>{}, &s);
}