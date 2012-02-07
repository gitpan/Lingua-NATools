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

#ifndef __INVINDEX_H__
#define __INVINDEX_H__

/**
 * @file
 * @brief Data structure for invertion indexes creation
 */

/** @brief 2<sup>24</sup>, the maximum value - 1 able to be stored in three bytes (24 bits) */
#define TWO_POWER_TWENTYFOUR 16777216

/** @brief list of characters used to ignore words in case of their existence */
#define IGNORE_WORDS ",.:;!?\"+-*/\\%^()[]@#=&%_"

/** @brief the size of the cell to be used on the linked list of occurrences */
#define CELLSIZE 50

#include "standard.h"
#include "bucket.h"

/**
 * @brief Structure for each word occurrence
 *
 * This structure stores a set of packed occurrences for a word.
 */
typedef struct cInvIndexEntry {
    /** buffer where the packed occurrences are stored */
    guint32* data;
    /** the size of the buffer (we normally use CELLSIZE) */
    guint32 size;
    /** the offset for the first free position  */
    guint32 ptr;
    /** linked list pointer for the next buffer cell  */
    struct cInvIndexEntry *next;
} InvIndexEntry;

/**
 * @brief Structure for the invertion index
 *
 * Main data structure for the invertion index creation. It is not
 * used to load invertion indexes from disk. For that use
 * CompactInvIndex.
 */
typedef struct cInvIndex {
    /** array size (number of words) */
    guint32 size;
    /** array usage */
    guint32 lastid;
    /** number of entries */
    guint32 nrentries;
    /** array list */
    struct cInvIndexEntry **buffer;
} InvIndex;

/**
 * @brief Compact structure for the invertion index
 */
typedef struct cCompactInvIndex {
    /** buffer for offsets for each word */
    guint32 *buffer;
    /** number of words (also, size of buffer) */
    guint32 nrwords;
    /** buffer for occurrences (size is nrwords + nrentries) */
    guint32 *entry;
    /** number of occurrences  */
    guint32 nrentries;
} CompactInvIndex;

InvIndex*        inv_index_new(
                         guint32 original_size);

InvIndex*        inv_index_add_occurrence(
                         InvIndex *index,
			 guint32 wid,
			 guchar  chunk,
			 guint32 sentence);

int              inv_index_save_hash(
                         InvIndex *index,
			 const gchar *filename,
			 gboolean quiet);

void             inv_index_free(
                         InvIndex *index);

CompactInvIndex *inv_index_compact_new(
                         guint32 nrwords,
			 guint32 nrentries);

CompactInvIndex *inv_index_compact_load(
                         const char* filename);

InvIndex*        inv_index_add_chunk(
                         InvIndex *index,
			 guchar chunk,
			 CompactInvIndex *cii);

void             inv_index_compact_free(
                         CompactInvIndex *cii);

guint32*         inv_index_compact_get_occurrences(
                         CompactInvIndex *index,
			 guint32          wid);

guint32          unpack( guint32          packed,
			 guchar          *character);

guint32          pack(   guint32          integer,
			 guchar           character);

guint32* intersect(guint32 *self, guint32 *other);

/* size_t inv_index_buffer_size(guint32 *buffer); */

#endif /* __INVINDEX_H__ */
