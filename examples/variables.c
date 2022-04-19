#include <SGL/CSGL.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>//malloc, free
#include <string.h>//memcpy
#include <stdio.h>

struct val_s {
    int32_t arr[5];
    double g;
};

typedef struct val_s val;

int main() {
    sgl_state s = sgl_new_state();

    //val type - same as val in custom_types_a.c
    sgl_type_member val_members[2];
    val_members[0] = sgl_create_type_member_buildin_t_array("arr", t_int32, offsetof(val, arr), 5);
    val_members[1] = sgl_create_type_member_buildin_t("g", t_float64, offsetof(val, g));
    sgl_register_struct(s, "val", sizeof(val), val_members, 2, 0, 0, 0);

    sgl_parse_result p = sgl_parse_file(s, "data/variables.sgl");
    
    val c = {0};
    sgl_get_local_value(p, "c", &c);
    printf("val c = {{%i, %i, %i, %i, %i}, %lf};\n", c.arr[0], c.arr[1], c.arr[2], c.arr[3], c.arr[4], c.g); 

    //TODO add array example

    sgl_delete_state(s);
    return 0;
}