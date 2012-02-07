/* -*- Mode: C; c-file-style: "stroustrup" -*- */

/* NATools - Package with parallel corpora tools
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

#ifndef __NATDICT_H__
#define __NATDICT_H__

#include <glib.h>

#include "dictionary.h"
#include "natlexicon.h"

/**
 * @file
 * @brief NATDict object API header file
 */

/**
 * @brief NATDict object structure
 */
typedef struct _NATDict {
    /** Dictionary source language */
    gchar      *source_language;
    /** Dictionary target language */
    gchar      *target_language;

    /** Dictionary source lexicon object */
    NATLexicon *source_lexicon;
    /** Dictionary target lexicon object */
    NATLexicon *target_lexicon;

    /** Dictionary from the source to the target language */
    Dictionary *source_dictionary;
    /** Dictionary from the target to the source language */
    Dictionary *target_dictionary;
} NATDict;


NATDict*           natdict_open              (const gchar   *filename);

gint32             natdict_save              (NATDict       *self,
					      const gchar   *filename);

NATDict*           natdict_new               (const gchar   *source_language,
					      const gchar   *target_language);

void               natdict_perldump          (NATDict       *self);

NATDict*           natdict_add               (NATDict       *dic1,
					      NATDict       *dic2);

guint32            natdict_id_from_word      (NATDict       *self,
					      gboolean       language,
					      const gchar   *word);

gchar*             natdict_word_from_id      (NATDict       *self,
					      gboolean       language,
					      guint32        id);

NATLexicon*        natdict_load_lexicon               (FILE            *fh);

guint32  natdict_word_count(NATDict *self, gboolean language, guint32 id);

float  natdict_dictionary_get_val(NATDict *self, gboolean language, guint32 wid, guint32 pos);
guint32  natdict_dictionary_get_id(NATDict *self, gboolean language, guint32 wid, guint32 pos);

#endif /* __NATDICT_H__ */
