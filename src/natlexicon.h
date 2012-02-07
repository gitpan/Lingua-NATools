/* -*- Mode: C; c-file-style: "stroustrup" -*- */

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

#ifndef __NATLEXICON_H__
#define __NATLEXICON_H__

#include <glib.h>

#include "dictionary.h"


/**
 * @file
 * @brief NATLexicon object API header file
 */

/**
 * @brief NATCell object structure
 */
typedef struct _NATCell {
    /** the offset of the word in the words string*/
    guint32 offset;
    /** occurrence count of the word in the corpus */
    guint32 count;
    /** word identifier (equal to the cell index in the main array) */
    guint32 id;
} NATCell;

/**
 * @brief NATLexicon object structure
 */
typedef struct _NATLexicon {
    /** offset of the end of the string in the words string array */
    guint32   words_limit;
    /** word string array, a collection of words separated by the NULL character */
    gchar    *words;

    /** array of word cells */
    NATCell  *cells;
    /** index for the next free cell on the array (number of elements in the array)*/
    guint32   count;
} NATLexicon;

guint32            natlexicon_id_from_word            (NATLexicon      *lexicon,
						       const gchar     *word);

gchar*             natlexicon_word_from_id            (NATLexicon      *lexicon, 
						       guint32          id);

guint32            natlexicon_count_from_id           (NATLexicon      *lexicon, 
						       guint32          id);


NATLexicon*        natlexicon_conciliate              (NATLexicon      *lex1,
						       guint32        **it1,
						       NATLexicon      *lex2, 
						       guint32        **it2);

NATCell*           natlexicon_search_word             (NATLexicon      *lex,
						       const gchar     *word);

void               natlexicon_free                    (NATLexicon      *self);

#endif
