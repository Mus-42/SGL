#include <SGL/SGL.hpp>
#include <SGL/CSGL.h>

#include <fstream>

extern "C" {   
    sgl_state sgl_new_state() {
        return new SGL::state;
    }
    void sgl_delete_state(sgl_state s) {
        delete reinterpret_cast<SGL::state*>(s);
    }
    sgl_parse_result sgl_new_parse_result() {
        return new SGL::parse_result;
    }
    void sgl_delete_parse_result(sgl_parse_result p) {
        delete reinterpret_cast<SGL::parse_result*>(p);
    }

    sgl_type_member sgl_create_type_member_buildin_t(const char* name, sgl_privitive_type type, size_t offset) {
        return sgl_create_type_member_buildin_t_array(name, type, offset, 0);
    }
    sgl_type_member sgl_create_type_member_custom_t(const char* name, const char* type_name, size_t offset) {
        return sgl_create_type_member_custom_t_array(name, type_name, offset, 0);
    }

    sgl_type_member sgl_create_type_member_buildin_t_array(const char* name, sgl_privitive_type type, size_t offset, size_t array_size) {
        sgl_type_member ret;
        ret.name = name;
        ret.custom_type_name = nullptr;
        ret.type = type;
        ret.m_type = nullptr;
        ret.offset = offset;
        ret.array_size = array_size;
        return ret;
    }
    sgl_type_member sgl_create_type_member_custom_t_array(const char* name, const char* type_name, size_t offset, size_t array_size) {
        sgl_type_member ret;
        ret.name = name;
        ret.custom_type_name = type_name;
        ret.type = t_custom;
        ret.m_type = nullptr;
        ret.offset = offset;
        ret.array_size = array_size;
        return ret;
    }

    void sgl_register_struct(sgl_state s, const char* name, size_t struct_size, const sgl_type_member* members, size_t members_count, 
        sgl_value_constructor constructor, sgl_value_destructor destuructor, sgl_value_copy copy) {     
        std::vector<SGL::type::member> m(members_count);
        for(size_t i = 0; i < members_count; i++) {
            m[i].type = static_cast<decltype(m[i].type)>(members[i].type);
            m[i].offset = members[i].offset;
            m[i].array_size = members[i].array_size;
            m[i].m_type = static_cast<decltype(m[i].m_type)>(members[i].m_type);
            m[i].name = members[i].name;
            if(members[i].custom_type_name) m[i].custom_type_name = members[i].custom_type_name;
        }
        SGL::details::register_struct(static_cast<SGL::state*>(s), name, struct_size, std::move(m), reinterpret_cast<void*>(constructor), reinterpret_cast<void*>(destuructor), reinterpret_cast<void*>(copy));
    }
      
    sgl_parse_result sgl_parce_file(sgl_state s, const char* filename) {
        std::ifstream in(filename);
        if(!in.is_open()) return nullptr;
        return static_cast<sgl_parse_result>(SGL::parse_stream(static_cast<SGL::state*>(s), in));
    }

    void sgl_get_local_value(sgl_parse_result p, const char* var_name, void* dest) {
        auto r = SGL::details::get_local_value(static_cast<SGL::parse_result*>(p), var_name);
        SGL::details::copy_val(r->m_type, 0, dest, r->data);
    }
}