#include <SGL/CSGL.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

struct val {
    int32_t arr[5];
    double g;
} val;

int main() {
    sgl_state s = sgl_new_state();

    sgl_type_member val_members[2];

    val_members[0] = sgl_create_type_member_buildin_t_array("arr", t_int32, offsetof(struct val, arr), 5);
    val_members[1] = sgl_create_type_member_buildin_t("g", t_float64, offsetof(struct val, g));

    sgl_register_struct(s, "val", sizeof(val), val_members, 2, 0, 0, 0);

    sgl_parse_result p = sgl_parce_file(s, "test1.sgl");

    double d = 0.;
    sgl_get_local_value(p, "d", &d);

    printf("doble d = %lf;", d);

    sgl_delete_state(s);//it also delete all parse results in state
    return 0;
}