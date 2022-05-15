#include <SGL/SGL.hpp>
#include <SGL/function.hpp>

#include <iostream>

int main() {
    //TODO add example

    //some test code

    //const SGL::type t_str(SGL::sgl_type_identity<std::string>{}, "string");
/*
    SGL::type t_int32(SGL::sgl_type_identity<int32_t>{}, "int32", nullptr);

    SGL::state s;
    auto& t_str = s.register_type<std::string>("string");   
    auto& t_float = s.register_type<float>("float");   
    auto& t_int = s.register_type<int>("int");   
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

    SGL::tokenizer tk("int a = b;//comment\nint c = 0;");
*/

    //SGL::value_type(SGL::sgl_type_identity<int * const[100]>{}, &s);
    //struct m_func {
    //    void operator()() const {
    //        std::cout << "fn call" << std::endl;
    //    };
    //};
    //std::function<void()> f = m_func();
    //f();

/*
    SGL::function func(std::function<void(int&, std::string&, std::vector<int>&)>([](int& a, std::string& s, std::vector<int>& vec) -> void {
        std::cout << "sgl function call: " << a << '\n';
        std::cout << "s: " << s << std::endl;
        std::cout << "my vec: [ ";
        for(auto v : vec) std::cout << v << ' ';
        std::cout << "]\n";
    }), &s);

    int a_arg_v = 100;
    std::string s_arg_v = "Mim OGD cstdiofan";
    std::vector<int> arg_vec_v = {1, 42, 26, 34, 5};

    SGL::value a_arg(SGL::sgl_type_identity<int&>{}, a_arg_v, s.get_value_type<int&>());
    SGL::value s_arg(SGL::sgl_type_identity<std::string&>{}, s_arg_v, s.get_value_type<std::string&>());

    SGL::value vec_arg(SGL::sgl_type_identity<std::vector<int>&>{}, arg_vec_v, s.get_value_type<std::vector<int>&>());

    func.call({a_arg, s_arg, vec_arg});


    std::cout << std::boolalpha << std::endl;

    std::cout << (typeid(int[]) == typeid(int*)) << std::endl;
    std::cout << (typeid(int[]) == typeid(int[])) << std::endl;
    std::cout << typeid(std::add_pointer_t<int[]>).name() << std::endl;
    std::cout << typeid(std::add_pointer_t<std::add_pointer_t<int[]>>).name() << std::endl;
    std::cout << typeid(int(**)[]).name() << std::endl;
    std::cout << typeid(int*[]).name() << std::endl;
*/
    //SGL::for_each_type_decorator(SGL::sgl_type_identity<float[100][12][4]>{}, [](auto t){
    //    std::cout << typeid(typename decltype(t)::type).name() << std::endl;
    //});
    //const int a[4][5] = {0};
    //std::cout << typeid(a[0]).name() << std::endl;
    //std::cout << typeid(a[0]).name() << std::endl;
    //std::cout << typeid(a[0]).name() << std::endl;

    //   std::cout << s.get_value_type<std::string const*&>().name_string() << std::endl;

    SGL::value val;

    auto d1 = new SGL::details::array_impl;
    d1->m_size = 4;
    auto d2 = new SGL::details::array_impl[4];
    d1->m_elements = d2;

    int d2_0[4] = {1, 2, 3, 4}; 
    d2[0].m_size = 4;
    d2[0].m_elements = d2_0;

    int d2_1[2] = {5, 6}; 
    d2[1].m_size = 2;
    d2[1].m_elements = d2_1;

    int d2_2[6] = {7, 8, 9, 10, 11, 12}; 
    d2[2].m_size = 6;
    d2[2].m_elements = d2_2;

    int d2_3[1] = {13}; 
    d2[3].m_size = 1;
    d2[3].m_elements = d2_3;

    val.m_data = d1;

    auto vec = val.get<std::vector<std::vector<int>>>();

    for(auto& el : vec) {
        for(auto v : el) std::cout << v << ' ';
        std::cout << '\n';
    }

}