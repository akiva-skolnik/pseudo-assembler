#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct Table Table;

/* getters */ 
char *get_data(const Table *node);
int get_address(const Table *node);
Table *get_next(const Table *node);

/* adding a line after node, and points node to the new line.
 * IMPORTANT! this function does not create a new copy of `data`, so you should free it using free_table only!
 */
Table *add_line(Table **node, char *data, int address);

/* inserts line in specific address (assuming you added lines in ascending order of address)
 * IMPORTANT! this function does not create a new copy of `data`, so you should free it using free_table only!
 */
void insert_line(Table *node, char *data, int address);

/* cleanup the table pointed to by p, including the data in it! */
void free_table(Table *p);

/* prints the table pointed to by p */
void print_table(const Table *p);
