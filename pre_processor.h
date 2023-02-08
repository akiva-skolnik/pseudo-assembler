#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/* \0 included */
#define MAX_WORD_LENGTH 31
/* https://opal.openu.ac.il/mod/ouilforum/discuss.php?d=3007542&p=7165520#p7165520 */
#define MAX_MACRO_LINES 6
#define START_MACRO "macro"
#define END_MACRO "endm"


typedef struct Macro Macro;


/* checks if the first word of every line is the start/end of macro definition or if it is an existing macro */
void pre_processor(const char *file_name, FILE *input);

/* saves everyting from the current location of input till END_MACRO as macro_name */
void save_macro(FILE *input, const char *macro_name);

/* get Macro by name */
Macro *get_macro(const char *macro_name);

/* cleanup space allocated at save_macro */
void free_macros(void);


