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

#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <glib.h>

/**
 * @file
 * @brief Header file for temporary dictionary data structure
 */

/**
 * @brief maximum number of entries in translation dictionary
 */
#define MAXENTRY 8

/**
 * @brief macro to access directly a word translation on the buffer
 */
#define DIC_POS(dic,word,j)   dic[word*MAXENTRY+j]

/**
 * @brief Per direction dictionary entries
 */
typedef struct dpair {
    /** translation word identifier  */
    guint32 id;
    /** translation probability  */
    float   val;
} DicPair;

/**
 * @brief Per direction dictionary structure
 */
typedef struct dictionary {
    /** dictionary information */
    DicPair *pairs;
    /** buffer with occurrences counts  */
    guint32 *occurs;
    /** number of dictionary entries  */
    guint32  size;
} Dictionary;

Dictionary*       dictionary_new                     (guint32       size);

void              dictionary_free                    (Dictionary   *dic);

Dictionary*       dictionary_set_id                  (Dictionary   *dic,
						      guint32       wid,
						      guint32       offset,
						      guint32       id);

Dictionary*       dictionary_set_val                 (Dictionary   *dic,
						      guint32       wid,
						      guint32       offset,
						      float         val);

Dictionary*       dictionary_set_occ                 (Dictionary   *dic,
						      guint32       wid,
						      guint32       count);

guint32           dictionary_get_occ                 (Dictionary   *dic,
						      guint32       wid);

guint32           dictionary_get_id                  (Dictionary   *dic,
						      guint32       wid,
						      guint32       offset);

float             dictionary_get_val                 (Dictionary   *dic,
						      guint32       wid,
						      guint32       offset);

guint32           dictionary_get_size                (Dictionary   *dic);

int               dictionary_save                    (Dictionary   *dic,
						      const char   *name);

int               dictionary_save_fh                 (FILE         *gzf,
						      Dictionary   *dic);

Dictionary*       dictionary_open                    (const char   *name);

Dictionary*       dictionary_load                    (FILE         *gzf);

Dictionary*       dictionary_add                     (Dictionary   *dic1,
						      Dictionary   *dic2);

double            dictionary_sentence_similarity     (Dictionary   *dic,
						      guint32      *s1,
						      int           s1size,
						      guint32      *s2,
						      int           s2size);

void              dictionary_remap                   (guint32      *Sit,
						      guint32      *Tit,
						      Dictionary   *dic);

void              dictionary_realloc                 (Dictionary   *dic,
						      guint32       nsize);

void              dictionary_realloc_map             (guint32      *Sit,
						      guint32      *Tit, 
						      Dictionary   *dic,
						      guint32       nsize);

#endif
