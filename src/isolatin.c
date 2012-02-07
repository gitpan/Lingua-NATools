/* NATools - Package with parallel corpora tools
 * Copyright (C) 1998-2001  Djoerd Hiemstra
 * Copyright (C) 2002-2004  Alberto Simões
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "isolatin.h"

/**
 * @file
 * @brief Code file to parse corpora using iso-8859-1*
 *
 * @todo Fix Documentation
 */


static int InWord(char ch)
{
    /* tokenising now by perl script */
    return (ch != ' ' && ch != '\n' && ch != '\t'); 
}

/**
 * @brief Searches begin of first word, skipping leading spaces, etc.
 *
 * @param text text to search
 * @param funct function to check if a char is from a word
 *
 * @return pointer to the beginning of the word on the text.
 */
static char *FirstTextWord(char *text, int (*funct)(char))

{
    while (*text != '\0' && !(*funct)(*text))
	text++;
    if (*text == '\0') return NULL;
    else return text;
}

/**
 * @brief Searches begin of next word, marking the current word with a \0 character 
 *
 * @param text text to search
 * @param funct function to check if a char is from a word
 *
 * @return pointer to the beginning of the word on the text
 */
static char *NextTextWord(char *text, int (*funct)(char))
{
    char* bow;
    bow = text;
    /* we are in the beginning of a word. Find its end! */
    while (*text != '\0' && (*funct)(*text))
	text++;
    /* if we end the buffer, return NULL */
    if (*text == '\0') return NULL;
    /* Mark end of the current word */
    *text++ = '\0';

    /* Search for the beginning of the next word */
    while (*text != '\0' && !(*funct)(*text))
	text++;
    /* if we end the buffer, return NULL */
    if (*text == '\0') return NULL;
    else return text;
}

/**
 * @brief ??
 *  
 * @param sen pointer to a buffer where the resulting sentence will be returned (list of words)
 * @param text pointer to a buffer where is the text to be searched
 * @param maxLen maximum size of the string
 * @param sd SoftDelimiter
 * @param hd HardDelimiter
 * @param funct function saying if a char is in a word, or not.
 */
static unsigned short NextTextString(char **sen, char **text, unsigned short maxLen,
				     char sd, char hd, int (*funct)(char))
{
    char *word;
    unsigned short len = 0;

    if (*text != NULL) {
	word = FirstTextWord(*text, funct);
	while (word != NULL && *word != sd) {
	    if (len < maxLen) {
		if (*word != hd) sen[(len)++] = word;
	    } 
	    else { 
		len ++;                              /* DUMMY stat */
	    }
	    word = NextTextWord(word, funct);
	}
	if (word != NULL)
	    word = NextTextWord(word, funct);
	if (word != NULL && *word == hd)
	    word = NextTextWord(word, funct);
	*text = word;
    }
    return len;
}

/**
 * @brief Gets a sentence at a time
 *
 * @param sen  pointer to buffer where sentence will be returned;
 * @param text pointer to the text where to search;
 * @param maxLen maximum size of the Sentence;
 * @param sd SoftDelimiter
 * @param hd HardDelimiter
 */
unsigned short NextTextSentence(char **sen, char **text, unsigned short maxLen, char sd, char hd)
{
    return NextTextString(sen, text, maxLen, sd, hd, InWord);
}

/**
 * @brief Reads all text from file to a text buffer
 *
 * @param filename Filename of the text file to be read
 * @return A big null-terminated buffer with the text file
 */
char *ReadText(const char *filename)
{
    FILE *fd;
    long len;
    struct stat stat_buf;
    char *result;
    
    fd = fopen(filename, "rt");
    if(fd == NULL) return NULL;
    if(fstat(fileno(fd), &stat_buf) == -1) return NULL;
    len = stat_buf.st_size;
    result = (char *) malloc(len+1);
    if(result == NULL) return NULL;
    if (fread(result, sizeof(char), len, fd) != len)
	return NULL;
    if (fclose(fd)) return NULL;
    result[len] = '\0';
    return(result);
}
