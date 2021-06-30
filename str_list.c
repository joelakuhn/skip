#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct str_list_node_t {
    char* str;
    struct str_list_node_t* next;
} str_list_node_t;

typedef struct {
    str_list_node_t* first;
    str_list_node_t* last;
} str_list_t;

str_list_t* str_list_new() {
    str_list_t* new_str_list = malloc(sizeof(str_list_t));
    new_str_list->first = NULL;
    new_str_list->last = NULL;
    return new_str_list;
}

void str_list_push(str_list_t* str_list, char* str) {
    str_list_node_t* new_node = malloc(sizeof(str_list_node_t));
    new_node->str = str;
    if (str_list->last == NULL) {
        str_list->first = new_node;
        str_list->last = new_node;
    }
    else {
        str_list->last->next = new_node;
        str_list->last = new_node;
    }
}

void str_list_flush(str_list_t* str_list, FILE* fh) {
    str_list_node_t* curr = str_list->first;
    while (curr != NULL) {
        str_list_node_t* next = curr->next;
        fprintf(fh, "%s", curr->str);
        free(curr->str);
        free(curr);
        curr = next;
    }
    str_list->first = NULL;
    str_list->last = NULL;
}
