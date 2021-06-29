#include <string.h>
#include <stdlib.h>

typedef struct {
    char** strs;
    int len;
    int allocated;
} str_vec_t;

str_vec_t* str_vec_new() {
    str_vec_t* str_vec = malloc(sizeof(str_vec_t));
    str_vec->strs = malloc(sizeof(char*) * 10);
    str_vec->allocated = 10;
    str_vec->len = 0;
    return str_vec;
}

void str_vec_push(str_vec_t* str_vec, char* str) {
    if ((str_vec->len + 1) >= str_vec->allocated) {
        str_vec->strs = (char**) realloc(str_vec->strs, (str_vec->allocated + 10) * sizeof(char*));
        str_vec->allocated += 10;
    }
    str_vec->strs[str_vec->len] = str;
    str_vec->len++;
}

void str_vec_flush(str_vec_t* str_vec, FILE* fh) {
    for (int str_i = 0; str_i < str_vec->len; str_i++) {
        fprintf(fh, "%s", str_vec->strs[str_i]);
        free(str_vec->strs[str_i]);
        str_vec->strs[str_i] = NULL;
    }
    str_vec->len = 0;
}
