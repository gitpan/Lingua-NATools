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

#include <glib.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <locale.h>
#include <string.h>

#include "standard.h"
#include "partials.h"


PartialCounts *PartialCountsAdd(PartialCounts *partials, guint32 wid)
{
    if (wid > partials->size) {
	guint32 newsize = partials->size;
	while(newsize < wid) { newsize*=1.2; }
	partials->buffer = (guint32*)g_realloc(partials->buffer, sizeof(guint32) * newsize);
	partials->size = newsize;
    }
    if (wid > partials->last) partials->last = wid;
    partials->buffer[wid]++;

    return partials;
}

void PartialCountsSave(PartialCounts *partials, const char* filename)
{
    FILE *fh = fopen(filename, "wb");
    guint32 x;
    if (!fh) report_error("Can't create partials counts");

    x = partials->last + 1;
    fwrite(&x, sizeof(guint32), 1, fh);
    fwrite(partials->buffer, sizeof(guint32), partials->last+1, fh);

    fclose(fh);
}

PartialCounts *PartialCountsLoad(const char* filename) 
{
    PartialCounts *partials;
    FILE *fh = fopen(filename, "rb");
    if (!fh) report_error("Can't load partials count");

    partials = g_new(PartialCounts, 1);

    fread(&partials->size, sizeof(guint32), 1, fh);
    partials->last = partials->size - 1;
    partials->buffer = g_new0(guint32, partials->size);
    fread(partials->buffer, sizeof(guint32), partials->size, fh);

    return partials;
}

void PartialCountsFree(PartialCounts *partials)
{
    g_free(partials->buffer);
    g_free(partials);
}
