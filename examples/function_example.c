#include <SGL/CSGL.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

sgl_cstring reverse_string(sgl_cstring str) {
    sgl_cstring ret;
    char* d = malloc(str.size + 1);
    for(size_t i = 0, s = str.size; i < s; i++) d[i] = str.data[s - i - 1];
    d[str.size] = '\0';
    ret.data = d;
    ret.size = str.size;
    return ret;
}

int main() {
    sgl_state s = sgl_new_state();

    sgl_parse_result p1 = sgl_parse_string(s, "double v = sqrt(2.) + pow(3., 1.5);");
    double v = 0.;
    sgl_get_local_value(p1, "v", &v);
    printf("double v = %lf;\n", v);

    sgl_primitive_type overload_args[1] = {t_cstring};
    sgl_function_overload overload;
    overload.ptr = reverse_string;
    overload.ret_type = t_cstring;
    overload.args_types_count = 1;
    overload.args_types = overload_args;
    sgl_function f = sgl_new_function(&overload, 1);
    sgl_add_function(s, "reverse_string", f);
    sgl_delete_function(f);

    sgl_parse_result p2 = sgl_parse_string(s, "string s = reverse_string(\"abcdefg\");");
    sgl_cstring str;
    sgl_get_local_value(p2, "s", &str);
    printf("string s = \"%s\";\n", str.data);

    sgl_delete_state(s);
    return 0;
}