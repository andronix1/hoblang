#include <stdio.h>
#include "core/vec.h"

int main() {
    char *asd = vec_new(char);
    vec_push(asd, 'H');
    vec_push(asd, 'e');
    vec_push(asd, 'l');
    vec_push(asd, 'l');
    vec_push(asd, 'o');
    vec_push(asd, '\n');
    vec_push(asd, '\0');
    puts(asd);
    vec_free(asd);

    char *a = "asd";
    printf("Hello, world!\n");
    return 0;
}
