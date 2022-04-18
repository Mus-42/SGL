//C Binding for SGL
#pragma once
#ifndef CSGL_INCLUDE_H_
#define CSGL_INCLUDE_H_ 1

#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

enum sgl_privitive_type {
    t_void = 0,
    t_int8, t_int16, t_int32, t_int64,
    t_uint8, t_uint16, t_uint32, t_uint64,
    t_float32, t_float64,
    t_bool,
    t_string,
    t_char,
    t_custom
};

//in C we work with poiters to C++ classes

typedef void* sgl_state;
typedef void* sgl_type;
typedef void* sgl_parse_result;

typedef void(*sgl_value_constructor)(void* value);
typedef void(*sgl_value_destructor)(void* value);
typedef void(*sgl_value_copy)(void* dst, void* srs);

typedef struct {//same as C++ SGL::type::member but use const char* instead of std::string
    sgl_privitive_type type;
    size_t offset, array_size;
    sgl_type m_type;
    const char* name;
    const char* custom_type_name;
} sgl_type_member;

sgl_state sgl_new_state();
void sgl_delete_state(sgl_state s);

sgl_parse_result sgl_new_parse_result();
void sgl_delete_parse_result(sgl_parse_result p);

sgl_type_member sgl_create_type_member_buildin_t(const char* name, sgl_privitive_type type, size_t offset);
sgl_type_member sgl_create_type_member_custom_t(const char* name, const char* type_name, size_t offset);

sgl_type_member sgl_create_type_member_buildin_t_array(const char* name, sgl_privitive_type type, size_t offset, size_t array_size);
sgl_type_member sgl_create_type_member_custom_t_array(const char* name, const char* type_name, size_t offset, size_t array_size);

//if your type don't have constructor|destructor|copy function - pass NULL. it will be generated automaticly
void sgl_register_struct(sgl_state s, const char* name, size_t struct_size, const sgl_type_member* members, size_t members_count, 
    sgl_value_constructor constructor, sgl_value_destructor destuructor, sgl_value_copy copy);

//copy variable to dest
void get_local_value(sgl_parse_result p, const char* var_name, void* dest);

#ifdef  __cplusplus
}//extern "C"
#endif//__cplusplus

#endif//CSGL_INCLUDE_H_