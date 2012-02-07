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

#ifndef __CORPUS_H__
#define __CORPUS_H__



#include "standard.h"

/**
 * @file
 * @brief Auxiliary data structure functions to encode a corpus header file
 */

/** The size of the header of the corpus file. */
#define CORPUS_HEADER_SIZE  sizeof(guint32)

/**
 * @brief Corpus word structure
 */
typedef struct cCorpusCell {
    /** word identifier */
    guint32 word;
    /** word flags. 1 means it is in UPPERCASE, 10 means it is Capitalized */
    gchar flags;
} CorpusCell;


/**
 * @brief Corpus structure
 */
typedef struct cCorpus {
    /** pointer to an array of words */
    CorpusCell *words;
    /** size of the corpus */
    guint32  length;
    /** pointer to the position being read */
    guint32  readptr;
    /** pointer to the write position */
    guint32  addptr;

    /** pointer to the direct access index of offsets*/
    guint32 *index;
    /** size of the direct access index */
    guint32  index_size;
    /** pointer to the write position in the direct access index  */
    guint32  index_addptr;
} Corpus;



Corpus*           corpus_new                      (void);

void              corpus_free                     (Corpus        *corpus);



int               corpus_add_word                 (Corpus        *corpus, 
						   guint32        word,
						   gint           flags);

CorpusCell*       corpus_first_sentence           (Corpus        *corpus);

CorpusCell*       corpus_next_sentence            (Corpus        *corpus);

guint32           corpus_sentence_length          (const CorpusCell *s);

int               corpus_load                     (Corpus        *corpus, 
						   const char    *filename);

int               corpus_save                     (Corpus        *corpus,
						   const char    *filename);

guint32           corpus_diff_words_nr            (Corpus        *corpus);

guint32           corpus_sentences_nr             (Corpus        *corpus);

guint32           corpus_sentences_nr_from_index  (char          *filename);

gboolean          corpus_strstr                   (const CorpusCell *haystack,
                                                   const guint32 *needle);



 


#endif /* __CORPUS_H__ */
