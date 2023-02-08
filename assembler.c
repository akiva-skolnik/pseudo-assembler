/* 
 * Version: 18/03/22
 *
 * This is an assambler for an imaginary assembly language.
 * For more info about that language, see the attached PDF.
 *
 * Usage: /.assembler file1 file2...
 * the assembler will compile file1.as, file2.as ... and will create 4 output files (in specific formats):
 * file.am -> the original file with all the macros replaced.
 * file.ent/ext -> all the .entry/.extern directive info will be there.
 * file.obj -> the translation of the assembly code to machine code.
 *
 */

#include "assembler.h"


/* global const sturct */
const struct {
    char command[5];
    int opcode;
    int funct;
    int operands;
    int legal_src_op[N_OPERANDS];
    int legal_dst_op[N_OPERANDS];
} cmds[] = {
    {"mov", 0, 0 , 2, {IMMEDIATE,DIRECT,INDEX,REGISTER}, {DIRECT,INDEX,REGISTER,INVALID}},
    {"cmp", 1, 0 , 2, {IMMEDIATE,DIRECT,INDEX,REGISTER}, {IMMEDIATE,DIRECT,REGISTER,REGISTER}},
    {"add", 2, 10, 2, {IMMEDIATE,DIRECT,INDEX,REGISTER}, {DIRECT,INDEX,REGISTER,INVALID}},
    {"sub", 2, 11, 2, {IMMEDIATE,DIRECT,INDEX,REGISTER}, {DIRECT,INDEX,REGISTER,INVALID}},
    {"lea", 4, 0 , 2, {DIRECT,INDEX,INVALID,INVALID},	 {DIRECT,INDEX,REGISTER,INVALID}},
    {"clr", 5, 10, 1, {INVALID,INVALID,INVALID,INVALID}, {DIRECT,INDEX,REGISTER,INVALID}},
    {"not", 5, 11, 1, {INVALID,INVALID,INVALID,INVALID}, {DIRECT,INDEX,REGISTER,INVALID}},
    {"inc", 5, 12, 1, {INVALID,INVALID,INVALID,INVALID}, {DIRECT,INDEX,REGISTER,INVALID}},
    {"dec", 5, 13, 1, {INVALID,INVALID,INVALID,INVALID}, {DIRECT,INDEX,REGISTER,INVALID}},
    {"jmp", 9, 10, 1, {INVALID,INVALID,INVALID,INVALID}, {DIRECT,INDEX,INVALID,INVALID}},
    {"bne", 9, 11, 1, {INVALID,INVALID,INVALID,INVALID}, {DIRECT,INDEX,INVALID,INVALID}},
    {"jsr", 9, 12, 1, {INVALID,INVALID,INVALID,INVALID}, {DIRECT,INDEX,INVALID,INVALID}},
    {"red", 12, 0, 1, {INVALID,INVALID,INVALID,INVALID}, {DIRECT,INDEX,REGISTER,INVALID}},
    {"prn", 13, 0, 1, {INVALID,INVALID,INVALID,INVALID}, {IMMEDIATE,DIRECT,INDEX,REGISTER}},
    {"rts", 14, 0, 0, {INVALID,INVALID,INVALID,INVALID}, {INVALID,INVALID,INVALID,INVALID}},
    {"stop",15, 0, 0, {INVALID,INVALID,INVALID,INVALID}, {INVALID,INVALID,INVALID,INVALID}}
};


struct This {
    char *file_name;
    int IC, DC, line_count;
    Table *data, *code, *missing_lines;
    bool error, has_entry, has_extern;
};


int main(int argc, char *argv[]) {
    FILE *file;
    This *this = malloc(sizeof(This));
    Table *data_head, *code_head, *missing_lines_head;
    while (--argc > 0)
        if ((file=open_file(this->file_name=argv[argc], ".as", "r")) == NULL)
            printf("ERROR: no such file %s.as\n", argv[argc]);
        else {
            pre_processor(this->file_name, file);
            
            this->IC = FIRST_ADDRESS;
            this->DC = 0;
            this->data = NULL;
            this->code = NULL;
            this->missing_lines = NULL;
            this->error = this->has_entry = this->has_extern = FALSE;
            /* dummy head to linked lists - it makes things simpler */
            data_head = add_line(&this->data, "", -1);
            code_head = add_line(&this->code, "", -1);
            missing_lines_head = add_line(&this->missing_lines, "", -1);
        
            file = open_file(this->file_name, ".am", "r");
            first_pass(this, file);
            fseek(file, 0, SEEK_SET);
            second_pass(this, file, code_head, missing_lines_head);
            fclose(file);

            /*
            printf("\nLABELS:\n");
            print_lables();
            printf("\nCODE:\n");
            print_table(code_head);
            printf("\nDATA:\n");
            print_table(data_head);
            printf("\nMISSING:\n");
            print_table(missing_lines_head);
            */
            
            if (!this->error) {
                /* skip dummy lines */
                write_obj(this, get_next(code_head), get_next(data_head));
                if (this->has_entry)
                    write_ent(this->file_name, get_next(missing_lines_head));
                if (this->has_extern)
                    write_ext(this->file_name, get_next(missing_lines_head));                
            }
            
            free_table(data_head);
            free_table(code_head);
            free_table(missing_lines_head);
            free_labels();
            if (this->error)
		printf("\n");  /* it looks better */
        }
    free(this);
    return 0;
}

/* parsing every line:
 * if there's label - save it with the right attr (or print warning).
 * if it is data/string directive - add memory word for each value/char.
 * if it is a command - see the comment at parse_command
 */
void first_pass(This *this, FILE *input) {
    int word_len;
    bool label_def, defined;
    char raw_line[LINE_LEN], *line;
    /* label and word will be used to temprary store single words */
    char *label = malloc(MAX_LABEL_LEN*sizeof(char)), *word = malloc(MAX_LABEL_LEN*sizeof(char));
    Label *lbl;
    for (this->line_count=1; fgets(raw_line, sizeof raw_line, input) != NULL; this->line_count++) {
        label_def = FALSE;
        line = trim(raw_line);
        if (line[0] == '\0' || line[0] == ';') /* comment or empty */
            continue;
        word = next_word(&line, word);
        word_len = strlen(word);
        label[0] = '\0';
        if (word[word_len-1] == ':') { /* label definition */
            word[--word_len] = '\0';
            label_def = TRUE;
            if (is_valid_label(word, &defined) && !is_cmd(word)) {
                strcpy(label, word);
            } else if (defined) {
                printf("%s.am:%d ERROR: multiple definitions of label %s\n", this->file_name, this->line_count, word);
                this->error = TRUE;
            }
            else {
                printf("%s.am:%d ERROR: invalid label %s\n", this->file_name, this->line_count, word);
                this->error = TRUE;
            }
            word = next_word(&line, word);
        }
        if (strcmp(word, ".data") == 0 || strcmp(word, ".string") == 0) {
            if (label[0] != '\0')
                save_label(label, this->DC, DATA);
            parse_directive(this, line, word);
        }

        else if (strcmp(word, ".extern") == 0 || strcmp(word, ".entry") == 0) {
            if (label_def)
                printf("%s.am:%d WARNING: label definition at %s directive\n", this->file_name, this->line_count, word);
            if (strcmp(word, ".extern") == 0) {
                word = next_word(&line, word);
                if (is_valid_label(word, NULL) && !is_cmd(word))
                    if ((lbl=get_label(word)) == NULL || label_attr(lbl, EXTERNAL)) {
                        save_label(word, 0, EXTERNAL);
                        this->has_extern = TRUE;
                    }
                    else {
                        printf("%s.am:%d ERROR: multiple definitions of label %s\n", this->file_name, this->line_count, word);
                        this->error = TRUE;
                    }
                        
                else {
                    printf("%s.am:%d ERROR: invalid label %s\n", this->file_name, this->line_count, word);
                    this->error = TRUE;
                }
            }
                
        } else { /* command */
            if (label[0] != '\0')
                save_label(label, this->IC, CODE);
            parse_command(this, line, word);
        }
    }

    free(label);
    free(word);
    update_data(this->IC);
}

/* - inserts base and offset values of label operands.
 * - updates entry attr for all labels (for the .ent file)
 */
void second_pass(This *this, FILE *input, Table *code_head, Table *missing_lines_head) {
    char raw_line[LINE_LEN], *line, *word = malloc(MAX_LABEL_LEN*sizeof(char));
    Label *lbl;
    Table *p = missing_lines_head;
    while (p != NULL) {
        if ((lbl=get_label(get_data(p))) != NULL) {
            insert_line(code_head, to_binary(BASE  (get_label_value(lbl)), NULL, BIN_W_LEN,
                        label_attr(lbl, EXTERNAL)?E:R), get_address(p));
            insert_line(code_head, to_binary(OFFSET(get_label_value(lbl)), NULL, BIN_W_LEN,
                        label_attr(lbl, EXTERNAL)?E:R), get_address(p)+1);
        }
        p = get_next(p);
    }

    for (this->line_count=1; fgets(raw_line, sizeof raw_line, input) != NULL && raw_line[0] != '\0'; this->line_count++) {
        line = trim(raw_line);
        word = next_word(&line, word);
        if (word[strlen(word)-1] == ':')  /* skip label def */
            word = next_word(&line, word);
        if (strcmp(word, ".entry") == 0) {
            lbl = get_label(word=next_word(&line, word));
            if (!is_valid_label(word, NULL) || is_cmd(word)) {
                printf("%s.am:%d ERROR: invalid label %s\n", this->file_name, this->line_count, word);
                this->error = TRUE;
            }
            else if (lbl == NULL)
                printf("%s.am:%d WARNING: label %s was not used\n", this->file_name, this->line_count, word);
            else if (label_attr(lbl, EXTERNAL)) {
                printf("%s.am:%d ERROR: both .entry and .extern have been used on label %s\n", this->file_name, this->line_count, word);
                this->error = TRUE;
            } else {
                set_attr(lbl, ENTRY);
                this->has_entry = TRUE;
            }
        }
    }
    free(word);
}


/* converts every char of .string and every int of .data to a binary line of machine code */
void parse_directive(This *this, char *line, const char *word) {
    int i, num;
    char *str, *p=malloc(10);
    if (line[0] == '\0') {
        printf("%s.am:%d ERROR: empty %s directive\n", this->file_name, this->line_count, word);
        this->error = TRUE;
    }
    if (strcmp(word, ".string") == 0) {
        if (line[0] != '"') {
            printf("%s.am:%d ERROR: missing opening quote at %s\n", this->file_name, this->line_count, word);
            this->error = TRUE;
            i = 0;
        } else
            i = 1;
        for (; line[i] != '\"' && line[i] != '\0' && memory_not_full(this); i++)
            if (isprint(line[i]))
                add_line(&this->data, to_binary(line[i], NULL , BIN_W_LEN, A), this->DC++);
            else {
                printf("%s.am:%d ERROR: invalid char number %d\n", this->file_name, this->line_count, line[i]);
                this->error = TRUE;
            }
        if (line[i] != '\"') {
            printf("%s.am:%d ERROR: missing closing quote at %s\n", this->file_name, this->line_count, word);
            this->error = TRUE;
        } else if (memory_not_full(this))
            add_line(&this->data, to_binary('\0', NULL, BIN_W_LEN, A), this->DC++);
    } else { /* .data */
        while (line[0] != '\0' && (str=get_operand(this, &line)) != NULL)
            if (sscanf(trim(str), "%d%s", &num, p)) {  /* %s to avoid float */
                if (memory_not_full(this))
                    add_line(&this->data, to_binary(num, NULL , BIN_W_LEN, A), this->DC++);
            }
            else {
                printf("%s.am:%d ERROR: %s is not a valid integer\n", this->file_name, this->line_count, trim(str));
                this->error = TRUE;
            }
    }
    free(p);
}


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
void parse_command(This *this, char *line, const char *word) {
    int i, temp, number1, number2, first_index_method, sec_index_method;
    char *memory_word;
    for (i=0; i < LENGTH(cmds) && strcmp(cmds[i].command, word) != 0; i++)
        ;
    if (i == LENGTH(cmds)) {
        printf("%s.am:%d ERROR: unknown command %s\n", this->file_name, this->line_count, word);
        this->error = TRUE;
    } else {  /* command found */
        memory_word = to_binary(0, NULL, BIN_W_LEN, A);
        memory_word[cmds[i].opcode] = '1';
        if (memory_not_full(this))
            add_line(&this->code, memory_word, this->IC++); /* opcode line */
        
        if (cmds[i].operands == 0 && line[0] != '\0') {
            printf("%s.am:%d ERROR: too many operands for %s\n", this->file_name, this->line_count, cmds[i].command);
            this->error = TRUE;
        } else if (cmds[i].operands > 0) {
            temp = this->IC; /* IC may change while checking the addressing_modes */
            memory_word = to_binary(0, NULL, BIN_W_LEN, A);
            to_binary(cmds[i].funct, memory_word+12, 4, NAN);
            
            first_index_method = INVALID;
            if (cmds[i].operands == 2) {
                first_index_method = addressing_modes(this, get_operand(this, &line),
                                                      memory_word, &number1, TRUE);
                to_binary(first_index_method, memory_word+6, 2, NAN);
                if (first_index_method != INVALID && !contain(cmds[i].legal_src_op, N_OPERANDS, first_index_method)) {
                    printf("%s.am:%d ERROR: invalid source operand for %s\n", this->file_name, this->line_count, cmds[i].command);
                    this->error = TRUE;
                }
            }
            sec_index_method = addressing_modes(this, get_operand(this, &line),
                                                memory_word, &number2, FALSE);
            to_binary(sec_index_method, memory_word+0, 2, NAN);
            if (memory_not_full(this)) {
                add_line(&this->code, memory_word, temp);
                this->IC++;
            }
            if (first_index_method == IMMEDIATE && memory_not_full(this))
                add_line(&this->code, to_binary(number1, NULL, BIN_W_LEN, A), this->IC++);
            if (sec_index_method   == IMMEDIATE && memory_not_full(this))
                add_line(&this->code, to_binary(number2, NULL, BIN_W_LEN, A), this->IC++);
                
            if (sec_index_method != INVALID && !contain(cmds[i].legal_dst_op, N_OPERANDS, sec_index_method)) {
                printf("%s.am:%d ERROR: illegal destinition operand for %s\n", this->file_name, this->line_count, cmds[i].command);
                this->error = TRUE;
            }

            if (line[0] != '\0') {
                printf("%s.am:%d ERROR: too many operands for %s\n", this->file_name, this->line_count, cmds[i].command);
                this->error = TRUE;
            }
        }
    }
}


/* converts a number n to binary string, and adds A,R,E value if its not NAN. */
char *to_binary(int n, char *binary, int width, ARE are) {
    int i;
    if (binary == NULL) {
        if (are != NAN) {
            binary = malloc(MEM_W_LEN);
            memset(binary+width, '0', MEM_W_LEN-BIN_W_LEN);
            binary[MEM_W_LEN-1] = '\0';
            binary[are] = '1';
        }
        else
            binary = malloc(width*sizeof(char));
    }
    for (i = 0; i < width; i++) /* https://opal.openu.ac.il/mod/ouilforum/discuss.php?d=3013154&p=7181831#p7181831 */
        binary[i] = (n >> i & 1) ? '1' : '0';
    return binary;
}

/* parsing operands seperated with a comma */
char *get_operand(This *this, char **line) {
    char *str = *line = trim(*line);
    int i, len = strlen(str);
    if (len == 0) {
	printf("%s.am:%d ERROR: missing operand\n", this->file_name, this->line_count);
	this->error = TRUE;
	str = NULL;
    } else if (str[0] == ',') {
        str = NULL;
        (*line)++;
        printf("%s.am:%d ERROR: operand starts with a commas\n", this->file_name, this->line_count);
        this->error = TRUE;
    } else if (str[len-1] == ','){
        str = NULL;
        (*line)[len-1] = '\0';
        printf("%s.am:%d ERROR: comma at the end of string\n", this->file_name, this->line_count);
        this->error = TRUE;
    } else {
        str[i=strcspn(str, ",")] = '\0'; /* replaces the first comma with \0 */
        *line = &str[i == len ? i : i+1]; /* i == len when there's no comma in str */
        str = trim(str);
        *line = trim(*line);
        if ((*line)[0] == ',') {
            (*line)++;
            printf("%s.am:%d ERROR: multiple concusive commas\n", this->file_name, this->line_count);
            this->error = TRUE;
        }
    }
    return str;
}


/* Identifying addressing modes and adding whatever possible to the machine code (or saving room for later).
 * Syntax:
 * IMMEDIATE - #<int>
 * DIRECT - <label>
 * INDEX - <label>[<register_name>]    where 10 <= register number <= 15
 * REGISTER - <register_name>
*/
int addressing_modes(This *this, const char *s, char *memory_word, int *number, bool is_source) {
    int index_method = INVALID;
    char *symbol = malloc(MAX_LABEL_LEN);
    if (s == NULL)
        ;
    else if (s[0] == '#') {
        index_method = IMMEDIATE;
        if (sscanf(s+1, "%d%s", number, symbol) == 1) /* %s to avoid float */
            memory_word[A] = '1';
        else {
            printf("%s.am:%d ERROR: %s is not a valid integer\n", this->file_name, this->line_count, s+1);
            this->error = TRUE;
        }
    } else if (sscanf(s, "r%d", number)) {
        index_method = REGISTER;
        if (*number >= 0 && *number <= 15)
            to_binary(*number, memory_word+(is_source?8:2), 4, NAN);
        else {
            printf("%s.am:%d ERROR: register should be between 0 and 15, not %d\n", this->file_name, this->line_count, *number);
            this->error = TRUE;
        }
    }
    else if (sscanf(s, "%[^[][r%d]", symbol, number) == 2) {
        index_method = INDEX;
        if (*number >= 10 && *number <= 15) {
            to_binary(*number, memory_word+(is_source?8:2), 4, NAN);
            add_line(&this->missing_lines, symbol, this->IC+1);
            this->IC += 2; /* save room for BASE + OFFSET */
        } else {
            printf("%s.am:%d ERROR: register index should be between 10 and 15, not %d\n", this->file_name, this->line_count, *number);
            this->error = TRUE;
        }
    }
    else if (is_valid_label(s, NULL) && !is_cmd(s)) {
        index_method = DIRECT;
        strcpy(symbol, s);
        add_line(&this->missing_lines, symbol, this->IC+1);
        this->IC += 2;
    } else {
        printf("%s.am:%d ERROR: invalid operand %s\n", this->file_name, this->line_count, s);
        this->error = TRUE;
    }
    
    if (index_method != INDEX && index_method != DIRECT)
        free(symbol);
    return index_method;
        
}

/* writing .obj file in specific hex-like format:
 * dividing each memory word to 5 groups, each starts with A,B... followed by hex number and seperated with a dash (-)
 */
void write_obj(This *this, Table *code_head, Table *data_head) {
    FILE *obj = open_file(this->file_name, ".obj", "w");
    Table *p = code_head;
    char hex[15];
    int i, j;
    fprintf(obj, "%d %d\n", this->IC - FIRST_ADDRESS, this->DC);
    for (i=FIRST_ADDRESS; i < (this->IC + this->DC); i++) {
        if (i == this->IC)
            p = data_head;
        sprintf(hex+1, "%lx", strtol(reversed(get_data(p)), NULL, 2));
        hex[0] = 'A';
        for (j=MEM_W_LEN/5; j > 0; j--) {
            hex[j*3+1] = hex[j+1];
            hex[j*3] = 'A' + j;
            hex[j*3-1] = '-';
        }
        fprintf(obj, "%04d %s\n", i, hex);
        p = get_next(p);
    }
    fclose(obj);
}

/* writing .ent files */
void write_ent(const char *file_name, Table *missing_lines_head) {
    FILE *ent = open_file(file_name, ".ent", "w");
    Label *p = get_first_label();
    while (p != NULL) {
        if (label_attr(p, ENTRY))
            fprintf(ent, "%s, %04d, %04d\n", get_label_symbol(p), BASE(get_label_value(p)), OFFSET(get_label_value(p)));
        p = get_next_label(p);
    }
    fclose(ent);
}

/* writing .ext files */
void write_ext(const char *file_name, Table *missing_lines_head) {
    FILE *ext = open_file(file_name, ".ext", "w");
    Label *p = get_first_label();
    Table *q;
    while (p != NULL) {
        if (label_attr(p, EXTERNAL)) {
            q = missing_lines_head;
            while (q != NULL) {
                if (strcmp(get_label_symbol(p), get_data(q)) == 0) {
                    fprintf(ext, "%s BASE %04d\n", get_data(q), get_address(q));
                    fprintf(ext, "%s OFFSET %04d\n", get_data(q), get_address(q)+1);
                }
                q = get_next(q);
            }
        }
        p = get_next_label(p);
    }
    fclose(ext);
}

/* returns whether the given string is a command */
bool is_cmd(const char *string) {
    int i;
    for (i=0; i < LENGTH(cmds) && strcmp(cmds[i].command, string) != 0; i++)
        ;
    return i != LENGTH(cmds);
}

/* returns true if the memory is not full */
bool memory_not_full(This *this) {
    if (this->IC + this->DC >= MEM_SIZE) {
        printf("%s.am:%d ERROR: memory is full", this->file_name, this->line_count);
        this->error = TRUE;
        return FALSE;
    } else 
        return TRUE;
    
}
