#include "pre_processor.h"

struct Macro {
    char name[MAX_WORD_LENGTH];
    char content[MAX_MACRO_LINES * LINE_LEN];
    struct Macro *next;
} *first_macro = NULL;

/* checks if the first word of every line is the start/end of macro definition or if it is an existing macro */
void pre_processor(const char *file_name, FILE *input) {
    FILE *output = open_file(file_name, ".am", "w");
    char raw_line[LINE_LEN], *line, *word = malloc(MAX_WORD_LENGTH * sizeof(char));
    Macro *m;
    while (fgets(raw_line, sizeof raw_line, input) != NULL) {
        line = trim(raw_line);
        if (line[0] == '\0')
            continue;
        word = next_word(&line, word);
        if (strcmp(word, START_MACRO) == 0)
            save_macro(input, next_word(&line, word));
        else if ((m=get_macro(word)) != NULL)
            fprintf(output, "%s", m->content);
        else
            fprintf(output, "%s %s\n", word, line);
    }
    free(word);
    free_macros();
    fclose(output);
}


/* saves everyting from the current location of input till END_MACRO as macro_name */
void save_macro(FILE *input, const char *macro_name) {
    int len = 0;
    char raw_line[LINE_LEN], *line;
    Macro *new_macro = (Macro*) malloc(sizeof(Macro));
    new_macro->next = NULL;
    strcpy(new_macro->name, macro_name);
    while (fgets(raw_line, sizeof raw_line, input) != NULL && raw_line[0] != '\0') {
        line = trim(raw_line);
        if (strncmp(line, END_MACRO, strlen(END_MACRO)) == 0)
            break;
        strcpy(&(new_macro->content[len]), line);
        len += strlen(line);
    }
        
    if (first_macro == NULL)
        first_macro = new_macro;
    else
        first_macro->next = new_macro;
}


/* get Macro by name */
Macro *get_macro(const char *macro_name) {
    Macro *m = first_macro;
    while (m != NULL && strcmp(m->name, macro_name) != 0)
        m = m->next;
    return m;
}


/* cleanup space allocated at save_macro */
void free_macros(void) {
    Macro *t, *m = first_macro;
    while (m != NULL) {
        t = m->next;
        free(m);
        m = t;
    }
    first_macro = NULL;
}





