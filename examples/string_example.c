#include <SGL/CSGL.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

struct val_with_string_s {
    int32_t i;
    sgl_cstring str;
    double d;
};

typedef struct val_with_string_s val_with_string;

int main() {
    sgl_state s = sgl_new_state();

    sgl_parse_result p1 = sgl_parse_string(s, "string s = \"ascii string\";");
    sgl_cstring str;
    sgl_get_local_value(p1, "s", &str);
    printf("string s = \"%s\";\n", str.data);

    sgl_type_member val2_members[3];
    val2_members[0] = sgl_create_type_member_buildin_t("i", t_int32, offsetof(val_with_string, i));
    val2_members[1] = sgl_create_type_member_buildin_t("fv", t_cstring, offsetof(val_with_string, str));
    val2_members[2] = sgl_create_type_member_buildin_t("fv", t_float64, offsetof(val_with_string, d));
    sgl_register_struct(s, "val_with_string", sizeof(val_with_string), val2_members, 3, 0, 0, 0);


    sgl_parse_result p2 = sgl_parse_string(s, "val_with_string q = {12, \"some text\", 14.26};");
    val_with_string q;
    sgl_get_local_value(p2, "q", &q);

    printf("val_with_string q = {%i, \"%s\", %lf};\n", q.i, q.str.data, q.d);

    sgl_delete_state(s);//it also delete all parse results in state
    return 0;
}