#include <iostream>
#include <fstream>

#include <SGL/SGL.hpp>

using namespace SGL;

struct all_ints {
    int8_t  i8_1,  i8_2;
    int16_t i16_1, i16_2;
    int32_t i32_1, i32_2;
    int64_t i64_1, i64_2;

    uint8_t  ui8_1,  ui8_2;
    uint16_t ui16_1, ui16_2;
    uint32_t ui32_1, ui32_2;
    uint64_t ui64_1, ui64_2;
};

struct val {
    int32_t arr[5];
    double g = 0;
};

struct example_struct {
    int32_t g;
    uint8_t q;
    float f;
    std::string s;
    val custom;
};

int main() {
    try
    {
        state s;
        /*
        register_struct<all_ints>(&s, "all_ints", {
            {"i8_1",  t_int8,  offsetof(all_ints, i8_1)},  {"i8_2",  t_int8,  offsetof(all_ints, i8_2)},
            {"i16_1", t_int16, offsetof(all_ints, i16_1)}, {"i16_2", t_int16, offsetof(all_ints, i16_2)},
            {"i32_1", t_int32, offsetof(all_ints, i32_1)}, {"i32_2", t_int32, offsetof(all_ints, i32_2)},
            {"i64_1", t_int64, offsetof(all_ints, i64_1)}, {"i64_2", t_int64, offsetof(all_ints, i64_2)},

            {"ui8_1",  t_uint8,  offsetof(all_ints, ui8_1)},  {"ui8_2",  t_uint8,  offsetof(all_ints, ui8_2)},
            {"ui16_1", t_uint16, offsetof(all_ints, ui16_1)}, {"ui16_2", t_uint16, offsetof(all_ints, ui16_2)},
            {"ui32_1", t_uint32, offsetof(all_ints, ui32_1)}, {"ui32_2", t_uint32, offsetof(all_ints, ui32_2)},
            {"ui64_1", t_uint64, offsetof(all_ints, ui64_1)}, {"ui64_2", t_uint64, offsetof(all_ints, ui64_2)},
        },
        [](all_ints* m){
            memset(m, 0, sizeof(all_ints));
            std::cout << "constucted all_ints value" << std::endl;
        },
        [](all_ints* m){
            std::cout << "destructed all_ints value" << std::endl;
        },
        [](all_ints* m, all_ints* q){
            memcpy(m, q, sizeof(all_ints));
            std::cout << "copied all_ints value" << std::endl;
        }
        );*/
        //*
        register_struct<val>(&s, "val", {
            {"arr", t_int32, offsetof(val, arr), 5},
            {"g", t_float64, offsetof(val, g)},
        });
        register_struct<example_struct>(&s, "example_struct", {
            {"g", t_int32, offsetof(example_struct, g)},
            {"q", t_uint8, offsetof(example_struct, q)},
            {"f", t_float32, offsetof(example_struct, f)},
            {"s", t_string, offsetof(example_struct, s)},
            {"custom", "val", offsetof(example_struct, custom)},
        });//*/
        std::ifstream in("test1.sgl");
        auto res = parse_stream(&s, in);
        in.close();

        //auto v = get_local_value<all_ints>(res, "i_v");
        double d = get_local_value<double>(res, "d");
/*
        double d = get_local_value<double>(res, "d");
        auto v = get_local_value<val>(res, "v");
        auto b = get_local_value<bool>(res, "b");
        //auto s1 = get_local_value<example_struct>(res, "s1");
        auto g = get_local_value<int8_t>(res, "g");
        auto s2 = get_local_value<example_struct>(res, "s2");
*/
    }
    catch(std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
    catch(...) {
        std::cout << "unknown error" << std::endl;
    }


    /*
    auto v = get_local_value<val>(res, "v");
    auto s1 = get_local_value<example_struct>(res, "s1");
    */

    //auto g = copy_local_value<int8_t>(res, "g");
    //auto s2 = copy_local_value<example_struct>(res, "s1");
}