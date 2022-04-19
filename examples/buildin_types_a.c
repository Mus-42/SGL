#include <SGL/CSGL.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

int main() {
    sgl_state s = sgl_new_state();//create state

    sgl_parse_result p1 = sgl_parse_file(s, "data/buildin_types_a.sgl");//parse file
    double d = 0.;
    sgl_get_local_value(p1, "d", &d);//get d value
    printf("doble d = %lf;\n", d);

    sgl_parse_result p2 = sgl_parse_string(s, "int a = 2 * 10 + 5;");//parse string
    int a = 0;
    sgl_get_local_value(p2, "a", &a);//get a value
    printf("int a = %i;\n", a);

    sgl_delete_state(s);//it also delete all parse results in state
    return 0;
}