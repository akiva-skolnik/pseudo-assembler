#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pre_processor.h"
#include "labels.h"
#include "table.h"
#include "util.h"

#define BASE(n) ((n)/16*16)
#define OFFSET(n) ((n) % 16)

#define MEM_SIZE 8192
#define FIRST_ADDRESS 100
#define BIN_W_LEN 16
#define MEM_W_LEN 20
#define N_OPERANDS 4

typedef struct This This;
typedef enum {A=18, R=17, E=16, NAN=0} ARE;
enum {IMMEDIATE, DIRECT, INDEX, REGISTER, INVALID=-2};



/* parsing every line:
 * if there's label - save it with the right attr (or print warning).
 * if it is data/string directive - add memory word for each value/char.
 * if it is a command - see the comment at parse_command
 */
void first_pass(This *this, FILE *input);
                
                
/* - inserts base and offset values of label operands.
 * - updates entry attr for all labels (for the .ent file)
 */
void second_pass(This *this, FILE *input, Table *code_head, Table *missing_lines_head);

/* converts every char of .string and every int of .data to a binary line of machine code */
void parse_directive(This *this, char *line, const char *word);

/* writing opcode line, and another line (if there's at lease one operand) at this format:
 * bits 0-1:    destinition addressing mode or zero if there's not dest op.
 * bits 2-5:    destinition register number or zero
 * bits 6-7:    source addressing mode or zero (if there's one operand - it is dest)
 * bits 8-11:   source register number or zero
 * bits 12-15:  funct in binary
 * bits 16-18:  A,R,E value
 * bit 19:      zero (always)
 *
 * if there's operand that's label, saves 2 lines for base and offset value (will be filled at the second pass)
 */
void parse_command(This *this, char *line, const char *word);
                   
/* converts a number n to binary string, and adds A,R,E value if its not NAN. */
char *to_binary(int n, char *binary, int width, ARE are);

/* parsing operands seperated with a comma */
char *get_operand(This *this, char **line);

/* Identifying addressing modes and adding whatever possible to the machine code (or saving room for later).
 * Syntax:
 * IMMEDIATE - #<int>
 * DIRECT - <label>
 * INDEX - <label>[<register_name>]    where 10 <= register number <= 15
 * REGISTER - <register_name>
*/
int addressing_modes(This *this, const char *s, char *memory_word, int *number, bool is_source);
                     
/* writing .obj file in specific hex-like format:
 * dividing each memory word to 5 groups, each starts with A,B... followed by hex number and seperated with a dash (-)
 */
void write_obj(This *this, Table *code_head, Table *data_head);

/* writing .ent file */
void write_ent(const char *file_name, Table *missing_lines_head);

/* writing .ext file */
void write_ext(const char *file_name, Table *missing_lines_head);

/* returns whether the given string is a command */
bool is_cmd(const char *string);

/* returns true if the memory is not full */
bool memory_not_full(This *this);

