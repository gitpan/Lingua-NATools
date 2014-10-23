/* -*- Mode: C; c-file-style: "stroustrup" -*- */

/* NATools - Package with parallel corpora tools
 * Copyright (C) 2002-2009  Alberto Sim�es
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

#ifndef __NGRAMIDX_H__
#define __NGRAMIDX_H__

#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <glib.h>

struct sqlite_struct {
    int n;
    sqlite3 * dbh;
    GHashTable * bigram_cache;
    GHashTable * trigram_cache;
    GHashTable * tetragram_cache;
};

typedef struct sqlite_struct SQLite;

SQLite* ngram_index_new(const char* filename, int n);
SQLite* ngram_index_open(const char* filename, int n);
SQLite* ngram_index_open_and_attach(const char* template);
void     ngram_index_close(SQLite* db);
void     bigram_add_occurrence(SQLite* db, guint32 w1, guint32 w2);
gboolean bigram_free_cache(gpointer key, gpointer value, gpointer user_data);
void     trigram_add_occurrence(SQLite* db, guint32 w1, guint32 w2, guint32 w3);
gboolean trigram_free_cache(gpointer key, gpointer value, gpointer user_data);
void     tetragram_add_occurrence(SQLite* db, guint32 w1, guint32 w2, guint32 w3, guint32 w4);
gboolean tetragram_free_cache(gpointer key, gpointer value, gpointer user_data);

#endif  /* __NGRAMIDX_H__ */
