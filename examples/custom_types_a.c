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

struct val2_s {
    int32_t int_value;
    int32_t unused_in_sgl_value;
    int* my_data;
    int data_size;
    float fv;
};

typedef struct val_s val;
typedef struct val2_s val2;

void val2_constructor_f(void* data) {
    val2* v = (val2*)data;
    v->int_value = 0;//default value
    v->unused_in_sgl_value = 245;

    v->my_data = malloc(sizeof(int) * 5);//allocate memory
    v->data_size = 5;

    v->fv = 0.f;

    puts("val2 constructed");
}

void val2_destructor_f(void* data) {
    val2* v = (val2*)data;
    free(v->my_data);//free pointer

    puts("val2 destructed");
}

void val2_copy_f(void* data, void* other) {
    val2* v = (val2*)data;
    val2* o = (val2*)other;

    *v = *o;//copy all values
    //copy data from pointer
    v->my_data = malloc(sizeof(int) * v->data_size);
    memcpy(v->my_data, o->my_data, sizeof(int) * v->data_size);

    puts("val2 copied");
}


int main() {
    sgl_state s = sgl_new_state();

    //val - simple type without constrictor | destructors | copy functions
    sgl_type_member val_members[2];
    val_members[0] = sgl_create_type_member_buildin_t_array("arr", t_int32, offsetof(val, arr), 5);
    val_members[1] = sgl_create_type_member_buildin_t("g", t_float64, offsetof(val, g));
    sgl_register_struct(s, "val", sizeof(val), val_members, 2, 0, 0, 0);

    
    sgl_type_member val2_members[2];
    val2_members[0] = sgl_create_type_member_buildin_t("int_value", t_int32, offsetof(val2, int_value));
    val2_members[1] = sgl_create_type_member_buildin_t("fv", t_float32, offsetof(val2, fv));
    sgl_register_struct(s, "val2", sizeof(val2), val2_members, 2, val2_constructor_f, val2_destructor_f, val2_copy_f);


    sgl_parse_result p = sgl_parse_file(s, "data/custom_types_a.sgl");
    
    val v;
    sgl_get_local_value(p, "v", &v);

    printf("val v = {{%i, %i, %i, %i, %i}, %lf};\n", v.arr[0], v.arr[1], v.arr[2], v.arr[3], v.arr[4], v.g);

    val2 v2 = {0};
    sgl_get_local_value(p, "v2", &v2);

    printf("val2 v2 = {%i, %f};\n", v2.int_value, v2.fv);

    free(v2.my_data); 

    sgl_delete_state(s);//it also delete all parse results in state
    return 0;
}