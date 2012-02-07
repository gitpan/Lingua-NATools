/* -*- Mode: C; c-file-style: "stroustrup" -*- */

/* NATools - Package with parallel corpora tools
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

#ifndef __CORPUSINFO_H__
#define __CORPUSINFO_H__

#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>



#include "parseini.h"
#include "words.h"
#include "invindex.h"
#include "corpus.h"
#include "dictionary.h"
#include "ngramidx.h"


typedef struct _CorpusChunks_ {
    guint32 *source_offset;
    guint32 *target_offset;
    guint32 size;
    FILE *source_crp;
    FILE *target_crp;
} CorpusChunks;

/* struct to encapsulate corpus information */
typedef struct _CorpusInfo_ {
    GHashTable *config;
    gchar      *filepath;

    gboolean standalone_dictionary;

    WordLstNode **SourceLexIds;
    WordList     *SourceLex;

    WordLstNode **TargetLexIds;
    WordList     *TargetLex;

    CompactInvIndex *SourceIdx;
    CompactInvIndex *TargetIdx;
    
    Dictionary* SourceTarget;
    Dictionary* TargetSource;

    SQLite* SourceGrams;
    SQLite* TargetGrams;
    
    guchar nrChunks;

    /* Offsets, all in memory */
    CorpusChunks *chunks;

    /* Offset caches */
    /*     guint32* offset_cache1; */
    /*     char* offset_cache_filename1; */
    /*     guint32* offset_cache2; */
    /*     char* offset_cache_filename2; */
    /*     int last_offset_cache; */

    /* Rank caches */
    double *rank_cache1;
    char* rank_cache_filename1;
    double *rank_cache2;
    char* rank_cache_filename2;
    int last_rank_cache;
    int has_rank;


} CorpusInfo;

CorpusInfo *corpus_info_new(char *filepath);

void LOG(char* log, ...);
void corpus_info_free(CorpusInfo *corpus);
gboolean corpus_info_has_ngrams(CorpusInfo *crp);

#endif
