#include <stdio.h>
#include <stdlib.h>
#include "words.h"

char* chomp(char *str) {
    char *rts = str;
    while(*str) {
	if (*str == '\n') {
	    *str = '\0';
	}
	str++;
    }
    return rts;
}

int main(void) {
    char buff[100];
    WordList *lst;
    FILE *fd;

    lst = word_list_new();
    printf("ok 1\n");

    fd = fopen("t/bin/words.input", "r");
    if (!fd) return 1;

    while(!feof(fd)) {
	fgets(buff, 100, fd);
	chomp(buff);
	if (!feof(fd)) {
	    word_list_add_word(lst, g_strdup(buff));
	}
    }
    fclose(fd);
    printf("ok 2\n");

    if (word_list_save(lst, "t/bin/words.output.bin") != TRUE) return 1;

    printf("ok 3\n");

    word_list_free(lst);

    lst = word_list_load("t/bin/words.output.bin", NULL);
    if (!lst) return 1;

    printf("ok 4\n");

    fd = fopen("t/bin/words.input", "r");
    while(!feof(fd)) {
	fgets(buff, 100, fd);
	if (!feof(fd)) {
	    if (word_list_get_id(lst, chomp(buff)) == 0) {
		fprintf(stderr, "Word %s returned 0\n", buff);
		return 1;
	    }
	}
    }
    printf("ok 5\n");

    fclose(fd);

    return 0;
}

