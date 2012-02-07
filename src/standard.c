/* NATools - Package with parallel corpora tools
 * Copyright (C) 1998-2001  Djoerd Hiemstra
 * Copyright (C) 2002-2009  Alberto Simões
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

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "standard.h"

/**
 * @file
 * @brief Utility functions code file
 */


struct timeval TIMER_BEFORE;
struct timeval TIMER_AFTER;


/**
 * @brief removes/truncates a string at the first new line
 *
 * @param str the string to be chomped
 *
 * @return the pointer to the same but changed string
 */
char* chomp(char *str)
{
    char *rts = str;
    while(*str) {
	if (*str == '\n' || *str == '\r') {
	    *str = '\0';
	    break;
	}
	str++;
    }
    return rts;
}


/**
 * @brief Reports an error and dies
 *
 * @param format The message error
 */
void report_error(const char *format, ...)
{
    va_list args;
    va_start (args, format);
    fprintf(stderr, "**ERROR** ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end (args);
    exit(2);
}



/**
 * @brief takes a string and dups it in Capitalized form
 *
 * @param  str the word to be capitalized
 * @return a newly string Capitalized
 */
char *capital_dup(const char* str)
{
    char *capital = g_strdup(str);
#if IGNORE_CASE
    /* ID: capital[0] = capital[0]; */
#else
    capital[0] = g_ascii_toupper(capital[0]);
#endif
    return capital;
}

/**
 * @brief takes a string and dups it in UPPERCASE form
 *
 * @param  str the word to be upper'ed
 * @return a newly string in uppercase
 */
char *uppercase_dup(const char* str)
{
    char *uppercase = g_strdup(str);
#if IGNORE_CASE
    /* ID :) */
#else
    char *ptr = uppercase;
    while(*ptr) {
	*ptr = g_ascii_toupper(*ptr);
	ptr++;
    }
#endif
    return uppercase;
}


/**
 * @brief detects is a word is Capitalized
 *
 * @param str the word to be checked
 * @return true if the word is Capitalized, false otherwise
 */
int isCapital(const char* str)
{
    int i = 0;
#if IGNORE_CASE
    return 0;
#else
    if (g_ascii_isupper(str[i])) {
	int ok = 1;
	i++;
	while(ok && str[i]) {
	    if (!g_ascii_islower(str[i])) ok = 0;
	    i++;
	}
	return ok;
    } else {
	return 0;
    }
#endif
}


/**
 * @brief detects is a word is in UPPERCASE
 *
 * @param str the word to be checked
 * @return true if the word is UPPERCASE, false otherwise
 */
int isUPPERCASE(const char* str)
{
#if IGNORE_CASE
    return 0;
#else
    int i = 0;
    int ok = 1;
    while(ok && str[i]) {
	if (!g_ascii_isupper(str[i])) ok = 0;
	i++;
    }
    return ok;
#endif
}
