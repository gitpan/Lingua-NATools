/* -*- Mode: C; c-file-style: "stroustrup" -*- */

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "words.h"

/**
 * @file
 * @brief Auxiliary data structure functions to collect words
 */


static int strcmpx(const char *s1,const char *s2)
{
    int result;
    do
        result = (*s1++) - (*s2++);
    while (!result && *s1 && *s2);
    if (*(s1-1) == '*' || *(s2-1) == '*') result = 0;
    if (!result && *s1) return 1;
    if (!result && *s2) return -1;
    else return result;
}

/**
 * @brief Creates a new word list object
 *
 * @return the newly word list or NULL in case of error
 */
WordList* word_list_new(void)
{
    WordList *list = g_new(WordList, 1);
    list->count = 1; 		/* FIRST IS THE NULL */
    list->occurrences = 0;
    list->tree = NULL;
    return list;
}


static WordLstNode* word_list_add_word_(WordLstNode* list, char *string, guint32* rn)
{
    int cmp;
    if (!list) {
        WordLstNode *cell = g_new(WordLstNode, 1);
        if (!cell) { *rn = 0; return list; }

        cell->string = string;
        cell->count = 1;        /* I would put 1... but original uses 2 */
        cell->left = NULL;
        cell->right = NULL;
        cell->id = *rn;
        return cell;
    } else {
        cmp = strcmpx(string, list->string);
        if (cmp < 0) {
            list->left = word_list_add_word_(list->left, string, rn);
        } else if (cmp > 0) {
            list->right = word_list_add_word_(list->right, string, rn);
        } else {
            *rn = list->id;
            list->count++;
        }
	return list;
    }
}



static WordLstNode* word_list_add_word_and_index_(WordLstNode* list, WordLstNode** index, 
						  char *string, guint32* rn)
{
    int cmp;
    if (!list) {
        WordLstNode *cell = g_new(WordLstNode, 1);
        if (!cell) { *rn = 0; return list; }

        cell->string = string;
        cell->count = 1;        /* I would put 1... but original uses 2 */
        cell->left = NULL;
        cell->right = NULL;
        cell->id = *rn;
	index[cell->id] = cell;
        return cell;
    } else {
        cmp = strcmpx(string, list->string);
        if (cmp < 0) {
            list->left = word_list_add_word_(list->left, string, rn);
        } else if (cmp > 0) {
            list->right = word_list_add_word_(list->right, string, rn);
        } else {
            *rn = list->id;
            list->count++;
        }
	return list;
    }
}

/**
 * @brief Adds a word to the word list object
 *
 * This function takes a word list object and a word. If the word does
 * not exist, a new cell is created and the new identifier
 * returned. If it already exists, the respective identifier is
 * returned.
 *
 * @param list the word list object
 * @param string the word being added
 * @return the identifier for that word
 */
guint32 word_list_add_word(WordList* list, char *string)
{
    guint32 register_number = list->count + 1;
    
    list->tree = word_list_add_word_(list->tree, string, &register_number);
    if (register_number == list->count+1) list->count++;
    list->occurrences++;

    return register_number;
}

/**
 * @brief FIXME
 */
guint32 word_list_add_word_and_index(WordList* list, WordLstNode** index, char *string)
{
    guint32 register_number = list->count + 1;
    
    list->tree = word_list_add_word_and_index_(list->tree, index, string, &register_number);
    if (register_number == list->count+1) list->count++;
    list->occurrences++;

    return register_number;
}


/**
 * @brief Returns the total number of words in the corpus
 *
 * @param list the word list object
 * @return the number of tokens in the corpus
 */
guint32 word_list_tokens_number(WordList* list)
{
    return list->occurrences;
}

/**
 * @brief Returns the number of words in a word list
 *
 * @param list the word list object
 * @return the number of the elements on the word list object
 */
guint32 word_list_size(WordList* list)
{
    return list->count;
}

/**
 * @brief Returns the number of tokens in the corpus
 *
 * @param list the word list object
 * @return the number of tokens in the corpus
 */
guint32 word_list_occurrences(WordList* list)
{
    return list->occurrences;
}



/**
 * @brief FIXME
 */
WordLstNode** word_list_enlarge(WordList* list, WordLstNode** wln, guint32 extracells)
{
    return g_realloc(wln, (extracells + list->count) * sizeof(WordLstNode*));
}

static void word_save_(WordLstNode* tree, FILE *fd)
{
    if (tree) {
        fwrite(&tree->id, sizeof(tree->id), 1, fd);
        fwrite(&tree->count, sizeof(tree->count), 1, fd);
        fwrite(tree->string, strlen(tree->string)+1, 1, fd);
        word_save_(tree->left, fd);
        word_save_(tree->right, fd);
    }
}

/**
 * @brief Saves a wordlist on a file
 *
 * @param list the word list object to be saved
 * @param filename a string with the name of the file being created
 * @return true unless the save process failed. In this case, false is returned.
 */
gboolean word_list_save(WordList* list, char* filename)
{
    FILE *fd;

    fd = fopen(filename, "w");
    if (fd == NULL)
        return FALSE;
    else {
        fwrite(&list->count,       sizeof(guint32), 1, fd);
        fwrite(&list->occurrences, sizeof(guint32), 1, fd);
        word_save_(list->tree, fd);
    }
    fclose(fd);
    return TRUE;
}


static WordLstNode* word_list_add_full_(WordLstNode* tree, guint32 id,
					guint32 count, char* string,
					WordLstNode ***ptr) {
    int cmp;
    if (!tree) {
        WordLstNode *cell = g_new(WordLstNode, 1);
        if (!cell) return tree;

        if (ptr && *ptr) {
            (*ptr)[id] = cell;
        }

        cell->string = string;
        cell->count = count;
        cell->left = NULL;
        cell->right = NULL;
        cell->id = id;
        
        return cell;

    } else {
        cmp = strcmpx(string, tree->string);
        if (cmp < 0) {
            tree->left = word_list_add_full_(tree->left, id, count, string, ptr);
            return tree;
        } else if (cmp > 0) {
            tree->right = word_list_add_full_(tree->right, id, count, string, ptr);
            return tree;
        } else {
            return tree;
        }
    }
}

/** 
 * @brief Adds a word in the tree maintaining a direct access array
 *
 * @param list the word list object
 * @param id the identifier for that word
 * @param count the occurrence count for that word
 * @param string the word
 * @param ptr a pointer to the direct access array
 * @return the word list object
 */
WordList* word_list_add_full(WordList* list, guint32 id,
			     guint32 count, const char* string, WordLstNode*** ptr)
{
    char *str = strdup(string);
    list->tree = word_list_add_full_(list->tree, id, count, str, ptr);
    list->count++;              /* we hope this is not called for two equal strings */
    list->occurrences+=count;
    return list;
}


/**
 * @brief Loads a word list object
 *
 * This function simply loads the file and returns the word
 * tree is the second argument passed is NULL.
 *
 * If the second argument is a valid pointer to a WordLstNode pointer
 * pointer (yeah, pointer of pointer is for girls. Real men use
 * pointer of pointer of pointer), when the function returns it will
 * contain a pointer to an array of pointer for direct access to tree
 * cells given its identifier. Use the function word_list_get_by_id() to
 * access to this object.
 *
 * @param filename filename of the word-list object
 * @param ptr a pointer to a pointer. See description
 * @return the loaded word-list object
 */
WordList* word_list_load(const char *filename, WordLstNode*** ptr)
{
    FILE *fd;
    int i;
    guint32 count;
    guint32 id, wc, tk;
    char buffer[MAXWORDLEN+1];
    char ch;
    
    WordList *tree;
    tree = word_list_new();
    
    fd = fopen(filename, "rb");
    
    if (fd == NULL)
        return NULL;
    else {
        if (!fread(&wc, sizeof(wc), 1, fd)) return NULL;
        if (!fread(&tk, sizeof(tk), 1, fd)) return NULL;

        if (ptr) {
            *ptr = g_new0(WordLstNode*, wc+1);
            if (!*ptr) return NULL;
        }
        
        while(!feof(fd)) {
            fread(&id, sizeof(id), 1, fd);
            if (!feof(fd)) {
                fread(&count, sizeof(count), 1, fd);
                i = 0;
                while ((ch = fgetc(fd))) {
                    buffer[i++] = ch;
                }
                buffer[i] = '\0';
                word_list_add_full(tree, id, count, buffer, ptr);
            }
        }
        fclose(fd);

        /* A little hack at the moment... */
        if (ptr) {
            WordLstNode *cell = g_new(WordLstNode, 1);
            cell->string = g_strdup("(none)"); 
            (*ptr)[1] = cell;
        }
	
        return tree;
    }
}


static void print_words_(WordLstNode *Words)
{
    if (Words) {
        print_words_(Words->left);
        printf(" '%s'\n", Words->string);
        print_words_(Words->right);
    }
}

/**
 * @brief prints the dictionary words to stdout
 *
 * This function is mainly used for debugging. Pass it a title and a
 * word list object and it will print them.
 *
 * @param title the title of the word list (for debug...)
 * @param lst the word list object
 */
void word_list_print(char* title, WordList *lst)
{
    printf("== %s ==\n", title);
    print_words_(lst->tree);
}

static void word_list_free_(WordLstNode *l) {
    if (l) {
        word_list_free_(l->left);
        word_list_free_(l->right);
        g_free(l);
    }
}

/**
 * @brief Frees the word list structure
 *
 * @param lst pointer to the tree to be freed.
 */
void word_list_free(WordList *lst)
{
    word_list_free_(lst->tree);
    g_free(lst);
}

/**
 * @brief Returns the word in the tree given the word identifier
 *
 * @param ptr auxiliary array of cell indexes
 * @param index word identifier (and index in the array of cells)
 * @return the word which identifier was supplied
 */
char* word_list_get_by_id(WordLstNode** ptr, guint32 index)
{
    WordLstNode *cell;
    cell = ptr[index];
    if (cell)
        return cell->string;
    else
	    return NULL;
}

/**
 * @brief Returns the full word cell in the tree given the word identifier
 *
 * @param ptr auxiliary array of cell indexes
 * @param index word identifier (and index in the array of cells)
 * @return the cell of that word in the tree.
 */
WordLstNode *word_list_get_full_by_id(WordLstNode  **ptr,
				      guint32        index)
{
    WordLstNode *cell;

    cell = ptr[index];
    if (cell)
        return cell;
    else
        return NULL;
}


/**
 * @brief Returns the number of occurrences from a word
 *
 * @param ptr auxiliary array of cell indexes
 * @param wid word identifier (and index in the array of cells)
 * @return the occurrence number for that word.
 */
guint32 word_list_get_count_by_id(WordLstNode **ptr, guint32 wid)
{
    WordLstNode *cell;
    /* FIXME: Check if wid is smaller to the size of the array */

    cell = ptr[wid];
    if (cell)
	return cell->count;
    else
	return 0;
}

/**
 * @brief Sets the number of occurrences for a word
 *
 * @param list the word list
 * @param ptr auxiliary array of cell indexes
 * @param wid word identifier (and index in the array of cells)
 * @return 0 on error.
 */
gint32 word_list_set_count_by_id(WordList *list, WordLstNode **ptr, guint32 wid, guint32 count)
{
    WordLstNode *cell;
    /* FIXME: Check if wid is smaller to the size of the array */

    cell = ptr[wid];
    if (cell) {
	guint32 ocount = cell->count;
        cell->count = count;
	list->occurrences = list->occurrences - ocount + count;
    } else 
        return 0;
    return 1;
}

static guint32 get_id_(WordLstNode *tree, const char *string) {
    if (tree) {
        int cmp = strcmpx(string, tree->string);
        if (cmp<0) {
            return get_id_(tree->left, string);
        } else if (cmp>0) {
            return get_id_(tree->right, string);
        } else {
            return tree->id;
        }
    } else {
        return 0;
    }
}

/**
 * @brief Returns the id for a specific word
 *
 * @param list WordList object to be searched
 * @param string word being searched
 * @return that word id
 */
guint32 word_list_get_id(WordList* list, const char *string)
{
    return  get_id_(list->tree, string);
}


