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

#ifndef __WORDS_H__
#define __WORDS_H__

#include "standard.h"

/** @brief maximum size for words in the corpus */
#define MAXWORDLEN 99


/**
 * @file
 * @brief Auxiliary data structure to collect words header file
 */


/**
 * @brief Cell in the tree of collected words
 *
 * WordLstNode is each cell in the tree os collected words (WordList)
 */
typedef struct cWordLstNode {
    /** word identifier */
    guint32  id;
    /** word occurrence counter */
    guint32  count;
    /** the word */
    char    *string;

    /** binary tree left branch */
    struct cWordLstNode *left;
    /** binary right left branch */
    struct cWordLstNode *right;
} WordLstNode; 

/**
 * @brief binary search tree for words collecting.
 *
 * WordNode is a binary search tree base structure, used for words
 * collecting.
 */
typedef struct cWordList {
    /** number of words in the tree and last identifier used */
    guint32          count;
    /** total number of occurrences (all cells count summed up) */
    guint32          occurrences;
    /** the binary search tree of words */
    WordLstNode     *tree;
} WordList;


WordList*              word_list_new                (void);

guint32                word_list_add_word           (WordList         *list, 
						     char             *string);

WordList*              word_list_add_full           (WordList         *list,
						     guint32           id,
						     guint32           count,
						     const char       *string,
						     WordLstNode    ***ptr);

WordList*              word_list_load               (const char       *filename,
						     WordLstNode    ***ptr);

void                   word_list_print              (char             *title,
						     WordList         *lst);

void                   word_list_free               (WordList         *lst);

char*                  word_list_get_by_id          (WordLstNode     **ptr,
						     guint32           index);

WordLstNode*           word_list_get_full_by_id     (WordLstNode     **ptr,
						     guint32           index);

guint32                word_list_size               (WordList         *list);

guint32                word_list_occurrences        (WordList         *list);

gboolean               word_list_save               (WordList         *list,
						     char             *filename);

guint32                word_list_get_id             (WordList         *lst,
						     const char       *string);

guint32                word_list_get_count_by_id    (WordLstNode     **ptr,
						     guint32           wid);

WordLstNode** word_list_enlarge(WordList* list, WordLstNode** wln, guint32 extracells);
#endif /* __WORDS_H__ */
