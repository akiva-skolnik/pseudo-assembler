#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* \0 included */
#define LINE_LEN 81

#define LENGTH(arr)  (sizeof(arr) / sizeof(*arr))

/* remove heading and trailing spaces from s, except for trailing \n */
char *trim (char *s);

/* reversing a string (s) in place */
char *reversed(char *s);

/* returns whether the array arr with length len contain the number n */
int contain(const int *arr, int len, int n);

/* return the first word of a line, and points the line to the next word */
char *next_word(char **line, char *word);

/* opens file_name.ext with the given mode*/
FILE *open_file(const char *file_name, const char *ext, const char *mode);