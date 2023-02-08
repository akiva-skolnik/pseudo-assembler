#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "util.h"

/* \0 included */
#define MAX_LABEL_LEN 31  

typedef enum {FALSE, TRUE} bool;
typedef enum {CODE, DATA, ENTRY, EXTERNAL} attribute;


typedef struct Label Label;


/* getters */
char *get_label_symbol(const Label *node);
int get_label_value(const Label *node);
bool label_attr(const Label *node, attribute attr);
Label *get_next_label(const Label *node);
Label *get_first_label(void);

/* get label by symbol */
Label *get_label(const char *symbol);

/* setters */
void set_attr(const Label *node, attribute attr);


/* checks if the given symbol (s) is a valid label.
 * pass boolean pointer if you want it to check if the symbol is already defined
 * (the result will be stored in that pointer)
 */
bool is_valid_label(const char *s, bool *defined);


/* inserts label to the beginning of the label table */
void save_label(const char *symbol, int value, attribute attr);

/* adds IC to each label with attr DATA (in order to separate the code from the data)*/
void update_data(int IC);

/* cleanup space allocated at save_label */
void free_labels(void);

/* prints all labels */
void print_lables(void);

