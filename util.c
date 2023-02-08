#include "util.h"


/* remove heading and trailing spaces from s, except for trailing \n */
char *trim (char *s) {
    int len, i, last_idx;
    if (s != NULL) {
        while (isspace(*s)) 
            s++;
        if ((len=strlen(s)) > 0) {
            last_idx = (s[len-1] == '\n')?2:1;
    
            for (i = len-last_idx; isspace(s[i]); i--)
                ;
            
            if (s[len-1] == '\n')
                s[i+1] = '\n';
            s[i+last_idx] = '\0';
        }
    }
    return s;
}


/* reversing a string (s) in place */
char *reversed(char *s) {
    int i, len = strlen(s);
    char temp;
    for (i=0; i<len/2; i++) {
        temp = s[i];
        s[i] = s[len-i-1];
        s[len-i-1] = temp;
    }
    return s;
}


/* returns whether the array arr with length len contain the number n */
int contain(const int *arr, int len, int n) {
    int i;
    for (i=0; i<len; i++)
        if (arr[i] == n)
            return 1;
    return 0;
}


/* return the first word of a line, and points the line to the next word */
char *next_word(char **line, char *word) {
    int i;
    if ((i=sscanf(*line, "%s %[^\n]", word, *line)) == EOF)
        word[0] = '\0';
    else if (i < 2)
        (*line)[0] = '\0';
    else
        *line = trim(*line);
    return trim(word);
}


/* opens file_name.ext with the given mode*/
FILE *open_file(const char *file_name, const char *ext, const char *mode) {
    FILE *f;
    int len = strlen(file_name);
    int ext_len = strlen(ext);
    char *fn = malloc(len+ext_len+1);
    strcpy(fn, file_name);
    strcpy(fn+len, ext);
    fn[len+ext_len] = '\0';
    f = fopen(fn, mode);
    free(fn);
    return f;
}

