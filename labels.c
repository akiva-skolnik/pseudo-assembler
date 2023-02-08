#include "labels.h"


struct Label {
    char *symbol;
    int value;  /* address of the label definition */
    attribute *attr;
    struct Label *next;
} *head = NULL;


/* getters */
char *get_label_symbol(const Label *node) {
    return node == NULL ? NULL : node->symbol;
}

int get_label_value(const Label *node) {
    return node == NULL ? -1 : node->value;
}

bool label_attr(const Label *node, attribute attr) {
    return node == NULL ? FALSE : node->attr[attr];
}

Label *get_next_label(const Label *node) {
    return node == NULL ? NULL : node->next;
}

Label *get_first_label(void) {
    return head;
}

/* get label by symbol */
Label *get_label(const char *symbol) {
    Label *p = head;
    if (symbol == NULL)
        return NULL;
    while (p != NULL && strcmp(p->symbol, symbol) != 0)
        p = p->next;
    return p;
}


/* setters */
void set_attr(const Label *node, attribute attr) {
    if (node != NULL)
        node->attr[attr] = TRUE;
}


/* checks if the given symbol (s) is a valid label.
 * pass boolean pointer if you want it to check if the symbol is already defined
 * (the result will be stored in that pointer)
 */
bool is_valid_label(const char *s, bool *defined) {
    int len;
    int i, r;
    if (s == NULL)
        return FALSE;
    len = strlen(s);
    if (defined != NULL) {
        if (get_label(s) != NULL) {
            *defined = TRUE;
            return FALSE;
        } else 
            *defined = FALSE;
    }
    for (i=0; i < len && isalnum(s[i]); i++)
        ;
    return len > 0 && len < MAX_LABEL_LEN && i == len && isalpha(s[0]) && (sscanf(s, "r%d", &r) != 1 || r > 15) &&
           strcmp(s, "data") && strcmp(s, "string") && strcmp(s, "entry") && strcmp(s, "extern") && 
           strcmp(s, "macro") && strcmp(s, "endm");
}

/* inserts label to the beginning of the labels table */
void save_label(const char *symbol, int value, attribute attr) {
    Label *node;
    node = malloc(sizeof(Label));
    node->value = value;
    node->symbol = malloc(MAX_LABEL_LEN);
    strcpy(node->symbol, symbol);
    node->attr = calloc(4, sizeof(attribute)); /* total 4 possible attributes - CODE, DATA, ENTRY, EXTERNAL */
    node->attr[attr] = TRUE;
    node->next = head;
    head = node;
}


/* adds IC to each label with attr DATA (in order to separate the code from the data)*/
void update_data(int IC) {
    Label *p = head;
    while (p != NULL) {
        if (p->attr[DATA])
            p->value += IC;
        p = p->next;
    }
}


/* cleanup space allocated at save_label */
void free_labels(void) {
    Label *q, *p = head;
    while (p != NULL) {
        q = p->next;
        free(p->symbol);
        free(p->attr);
        free(p);
        p = q;
    }
    head = NULL;
}


/* prints all labels */
void print_lables(void) {
    Label *p = head;
    printf("\nSYMBOL, VALUE\n");
    while (p != NULL) {
        printf("%s, %d\n", p->symbol, p->value);
        p = p->next;
    }
}


