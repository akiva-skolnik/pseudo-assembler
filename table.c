#include "table.h"

struct Table {
    char *data;
    int address;
    struct Table *next;
};

/* getters */
char *get_data(const Table *node) {
    return node == NULL ? NULL : node->data;
}

int get_address(const Table *node) {
    return node == NULL ? -1 : node->address;
}

Table *get_next(const Table *node) {
    return node == NULL ? NULL : node->next;
}


/* adding a line after node, and points node to the new line.
 * IMPORTANT! this function does not create a new copy of `data`, so you should free it using free_table only!
 */
Table *add_line(Table **node, char *data, int address) {
    Table *next = malloc(sizeof(Table));
    next->data = data;
    next->address = address;
    next->next = NULL;
    
    if (*node != NULL)
        (*node)->next = next;
    return *node = next;
}

/* inserts line in specific address (assuming you added lines in ascending order of address)
 * IMPORTANT! this function does not create a new copy of `data`, so you should free it using free_table only!
 */
void insert_line(Table *node, char *data, int address) {
    Table *next = malloc(sizeof(Table));
    Table *p = node;
    next->data = data;
    next->address = address;
    while (p != NULL && p->address < address) {
        node = p;
        p = p->next;
    }
    if (node != NULL) {
        next->next = node->next;
        node->next = next;
    }
}


/* cleanup the table pointed to by p, including the data in it! */
void free_table(Table *p) {
    Table *q;
    while (p != NULL) {
        q = p->next;
        if (p->data[0] != '\0') /* skip the dummy head */
            free(p->data);
        free(p);
        p = q;
    }
}


/* prints the table pointed to by p */
void print_table(const Table *p) {
    printf("\nADDRESS, DATA\n");
    while (p != NULL) {
        printf("%d, %s\n", p->address, p->data);
        p = p->next;
    }
}


