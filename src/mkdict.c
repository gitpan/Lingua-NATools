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


#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <string.h>

#include "words.h"
#include "dictionary.h"
#include "natdict.h"

/**
 * @file
 * @brief Based on lexicon files and temporary dictionaries, creates a
 * NATools dictionary
 */

static guint32 put_string(gchar *str, gchar *orig, guint32 ptr, guint32 size) {
    guint i = 0;
    while(orig[i]) {
	str[ptr++] = orig[i++];
	if (ptr == size) {
	    str = g_realloc(str, size + 32000);
	    g_message("** REALLOC **");
	}
    }
    str[ptr++] = '\0';
    return ptr;
}

static guint32 tree_to_array(guint32 nrwords, gchar *str, NATCell *cells, WordLstNode *tree, guint32 ptr,
		      guint32 size, guint32 *cellptr, guint32 *tab) {
    guint32 offset = ptr;
    if (tree->left)
	offset = tree_to_array(nrwords, str, cells, tree->left, offset, size, cellptr, tab);

    cells[*cellptr].offset = offset;
    cells[*cellptr].count = tree->count;
    cells[*cellptr].id = *cellptr;

#ifdef DEBUG
    if (tree->id >= nrwords || *cellptr >= nrwords) 
	g_message("*** Words/id/cellptr (%u,%u,%u) ***", nrwords, tree->id, *cellptr);
#endif
    tab[tree->id] = *cellptr;

    ++*cellptr;

    offset = put_string(str, tree->string, offset, size);

    if (tree->right)
	offset = tree_to_array(nrwords, str, cells, tree->right, offset, size, cellptr, tab);
    return offset;
}

static int save(char *outfile,
		char *lang1, char *lang2,
		char *dicfile1, guint32 *tab1,
		char *dicfile2, guint32 *tab2,
		char *string1, guint32 ptr1, NATCell *cells1, guint32 size1,
		char *string2, guint32 ptr2, NATCell *cells2, guint32 size2) {
    FILE *out = NULL;
    gint32 s;

    Dictionary *dic;

    out = gzopen(outfile, "wb");
    if (!out) return 0;

    // Say this is a NATDict
    gzprintf(out,"!NATDict");

    s = strlen(lang1)+1;
    gzwrite(out, &s, sizeof(gint32));
    gzwrite(out, lang1, s);

    s = strlen(lang2)+1;
    gzwrite(out, &s, sizeof(gint32));
    gzwrite(out, lang2, s);

    // Save first lexicon
    g_message("** Saving source Lexicon **");
    gzwrite(out, &ptr1, sizeof(guint32));
    gzwrite(out, string1, ptr1);
    gzwrite(out, &size1, sizeof(guint32));
    g_message("\tSize: %u", size1);
    gzwrite(out, cells1, sizeof(NATCell) * size1);

    // Save second lexicon
    g_message("** Saving target Lexicon **");
    gzwrite(out, &ptr2, sizeof(guint32));
    gzwrite(out, string2, ptr2);
    gzwrite(out, &size2, sizeof(guint32));
    g_message("\tSize: %u", size2);
    gzwrite(out, cells2, sizeof(NATCell)* size2);

    // Load first dictionary
    g_message("** Source -> Target dictionary **");
    g_message("\tLoading...");
    dic = dictionary_open(dicfile1);

    dictionary_remap(tab1, tab2, dic);

    g_message("\tSaving...");
    gzwrite(out, &dic->size, sizeof(guint32));
    gzwrite(out, dic->pairs, sizeof(DicPair)*MAXENTRY*(dic->size+1));
    gzwrite(out, dic->occurs, sizeof(guint32)*(dic->size+1));
    dictionary_free(dic);

    // Load second dictionary
    g_message("** Target -> Source dictionary **");
    g_message("\tLoading...");
    dic = dictionary_open(dicfile2);

    dictionary_remap(tab2, tab1, dic);

    g_message("\tSaving...");
    gzwrite(out, &dic->size, sizeof(guint32));
    gzwrite(out, dic->pairs, sizeof(DicPair)*MAXENTRY*(dic->size+1));
    gzwrite(out, dic->occurs, sizeof(guint32)*(dic->size+1));
    dictionary_free(dic);

    // Close the file
    g_message("** DONE **");
    gzclose(out);

    return 1;
}

static void go(char *lang1, char *lang2,
	char *lexfile1, char *dicfile1,
	char *lexfile2, char *dicfile2,
	char *outfile) {
    WordList *lex1, *lex2;
    gchar *string1 = NULL;
    gchar *string2 = NULL;
    guint32 ptr1 = 0;
    guint32 ptr2 = 0;
    guint32 size1, size2;
    NATCell *cells1 = NULL;
    NATCell *cells2 = NULL;
    guint32 cellptr1 = 0;
    guint32 cellptr2 = 0;
    guint32 *tab1 = NULL;
    guint32 *tab2 = NULL;

    /* ---- First ------------------------------- */
    lex1 = word_list_load(lexfile1, NULL);
    if (!lex1) { g_warning("Error loading lexicon 1"); exit(1); }

    size1 = 11 * lex1->count;

    string1 = g_new0(gchar, size1);
    if (!string1) { g_warning("Error allocating string1"); exit(1); }

    cells1 = g_new0(NATCell, lex1->count + 1);
    if (!cells1) { g_warning("Error allocating cells1"); exit(1); }

    tab1 = g_new0(guint32, lex1->count + 1);
    if (!tab1) { g_warning("Error allocating tab1"); exit(1); }

    tab1[0] = tab1[1] = lex1->count-1;
    ptr1 = tree_to_array(lex1->count,string1, cells1, lex1->tree, ptr1, size1, &cellptr1, tab1);
    
    cells1[cellptr1].offset = ptr1;
    cells1[cellptr1].count = 0;
    cells1[cellptr1].id = cellptr1;
    cellptr1++;

    g_message("** Preparing source Lexicon **");
    g_message("\tPtr is at %u and original size was %u", ptr1, size1);
    g_message("\tOffset on the array is %u", cellptr1);
    g_message("\tNULL is pointing to %u", tab1[0]);

    /* ---- Second ------------------------------ */
    lex2 = word_list_load(lexfile2, NULL);
    if (!lex2) { g_warning("Error loading lexicon 2"); exit(1); }

    size2 = 11*lex2->count;

    string2 = g_new0(gchar, size2);
    if (!string2) { g_warning("Error allocating string2"); exit(1); }

    cells2 = g_new0(NATCell, lex2->count+1);
    if (!cells2) { g_warning("Error allocating cells2"); exit(1); }

    tab2 = g_new0(guint32, lex2->count+1);
    if (!tab2) { g_warning("Error allocating tab2"); exit(1); }

    tab2[0] = tab2[1] = lex2->count-1;
    ptr2 = tree_to_array(lex2->count,string2, cells2, lex2->tree, ptr2, size2, &cellptr2, tab2);

    cells2[cellptr2].offset = ptr2;
    cells2[cellptr2].count = 0;
    cells2[cellptr2].id = cellptr2;
    cellptr2++;

    g_message("** Preparing target Lexicon **");
    g_message("\tPtr is at %u and original size was %u", ptr2, size2);
    g_message("\tOffset on the array is %u", cellptr2);
    g_message("\tNULL is pointing to %u", tab2[0]);

    save(outfile,
	 lang1, lang2, 
	 dicfile1, tab1, dicfile2, tab2,
	 string1, ptr1, cells1, cellptr1,
	 string2, ptr2, cells2, cellptr2);   

/*    save(outfile, lang1, lang2, 
	 dicfile1, tab1, dicfile2, tab2,
	 string1, ptr1, cells1, lex1->count,
	 string2, ptr2, cells2, lex2->count);   */


    word_list_free(lex1);
    word_list_free(lex2);
}

/**
 * @brief Main Program
 *
 * @todo Document all this program
 */
int main(int argc, char *argv[]) {
    
    if (argc != 8) {
	printf("mkdict <lang1> <lang2> <lex1> <dic1> <lex2> <dic2> <outfile>\n");
	return 1;
    } else {
	go(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
	return 0;
    }
}



