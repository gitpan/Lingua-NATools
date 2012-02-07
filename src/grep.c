/*
# NATools - Package with parallel corpora tools
# Copyright (C) 1998-2001  Djoerd Hiemstra
# Copyright (C) 2002-2009  Alberto Simões
#
# This package is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>




#include "words.h"
#include "corpus.h"
#include "invindex.h"

/**
 * @file
 * @brief Main program to grep sentence pairs on parallel corpora 
 *        using a set of words
 */


static int corpus_print_sentence(const char* filename,
				 guint32 *offsets,
				 WordLstNode **W,
				 guint32 sentence)
{
    FILE *fh;
    guint32 begin, end, delta;
    CorpusCell *buf, *ptr;
    
    fh = fopen(filename, "r");
    if (!fh) {
	fprintf(stderr,"error opening filename %s\n", filename);
	return 1;
    }

    begin = offsets[sentence];
    end = offsets[sentence+1];
    delta = end - begin;
    /* printf("DELTA: %u\n", delta); */

    buf = g_new0(CorpusCell, delta);
    fseek(fh, begin*sizeof(CorpusCell) + CORPUS_HEADER_SIZE, 0);
    fread( buf, sizeof(CorpusCell),delta,fh);

    ptr = buf;
    while(ptr->word) {
	char *word = word_list_get_by_id(W, ptr->word);
	if (ptr->flags & 0x1) {
	    word = uppercase_dup(word);
	} else if (ptr->flags & 0x2) {
	    word = capital_dup(word);
	} else {
	    word = g_strdup(word);
	}
	printf("%s ", word);
	free(word);

	ptr++;
    }
    g_free(buf);

    fclose(fh);

    printf("\n");
    return 0;
}


static guint32* load_offsets(char *filename, guint* size) {
    gchar *file = g_strdup_printf("%s.index", filename);    
    guint32 x;
    guint32 *bf;
    FILE *fd = fopen(file, "r");
    g_free(file);
    if (!fd) return NULL;

    fread(&x, sizeof(guint32), 1, fd);
    if (size) *size = x;
    bf = g_new(guint32, x);
    fread(bf, sizeof(guint32), x, fd);
    fclose(fd);
    return bf;
}



/**
 * @brief The main function 
 *
 * @todo Document this
 */
int main(int argc, char *argv[])
{
    char *source_lex, *target_lex, *source_idx, *word;
    WordLstNode **SourceIds;
    WordList *Source;
    WordLstNode **TargetIds;
    WordList *Target;
    CompactInvIndex *idx;
    int xpto = 0;
    guint32 wid;
    guint32 *occs, *occs1, *occs2;
    guint32 c = 0;

    if (argc < 5) {
	printf("Usage: nat-grep <source_lex> <target_lex> <source_idx> <word>*\n");
	return 0;
    }

    source_lex = argv[1];
    fprintf(stderr,"SourceLex: %s\n", argv[1]);
    Source = word_list_load(source_lex, &SourceIds);

    target_lex = argv[2];
    fprintf(stderr,"TargetLex: %s\n", argv[2]);
    Target = word_list_load(target_lex, &TargetIds);

    source_idx = argv[3];
    fprintf(stderr,"InvIndex: %s\n", argv[3]);
    idx = inv_index_compact_load(source_idx);

    if (!idx) {
	printf(" ** ERROR ** Can't read idx\n");
	exit(1);
    }

    fprintf(stderr, "Searching...\n");

    word = argv[4];
    
    wid = word_list_get_id(Source, word);

    /* printf("Wid: %u\n", wid); */
    /* fprintf(stderr,"Word: %s\n", word_list_get_by_id(SourceIds, wid)); */
    
    occs = inv_index_compact_get_occurrences(idx, wid);
    if (argc > 5) {
	argc-=5;
	while(argc) {
	    word = argv[4+argc];
	    wid = word_list_get_id(Source, word);
	    /* fprintf(stderr,"Word: %s\n", word_list_get_by_id(SourceIds, wid)); */
	    occs1 = inv_index_compact_get_occurrences(idx, wid);
	    occs2 = intersect(occs, occs1);
	    if (xpto) g_free(occs);
	    occs = occs2;
	    argc--;
	    xpto++;
	}
    }

    fprintf(stderr, "Retrieving...\n");

    while(*occs) {
	guchar  chunk;
	char source_filename[100];
	char target_filename[100];
	guint32 sentence = unpack(*occs, &chunk);
	guint32 size;
	guint32 *source_offsets;
	guint32 *target_offsets;

	/* printf("Sentence: %u   Chunk: %hhu\n", sentence, chunk); */

 	sprintf(source_filename, "source.%03hhu.crp", chunk); 
 	source_offsets = load_offsets(source_filename, &size); 

 	sprintf(target_filename, "target.%03hhu.crp", chunk); 
 	target_offsets = load_offsets(target_filename, &size); 


	if (corpus_print_sentence(source_filename, source_offsets, SourceIds, sentence-1)) {
	    report_error("Can't get the sentence");
	}

	if (corpus_print_sentence(target_filename, target_offsets, TargetIds, sentence-1)) {
	    report_error("Can't get the sentence");
	}

	printf("\n");

	g_free(source_offsets);
	g_free(target_offsets);

	occs++;
	c++;
    }

    fprintf(stderr,"Nr Occs: %u\n", c);

    return 0;
}


