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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "words.h"
#include "corpus.h"
#include "standard.h"
#include "isolatin.h"

/**
 * @file
 * @brief Corpora sentence searcher
 *
 * @todo Change or delete this code. The new grep.c file can be the
 * new implementation.
 */

/**
 * @brief Maximum sentence size (characters)
 */
#define MAXBUF 300

int asprintf(char **strp, const char *fmt, ...);

static void print_sentence(Corpus *crp, WordLstNode **W,
			   guint32 *offsets, guint32 nsen, gboolean show_ids) {
    CorpusCell *x;
    guint32 i;
    if (offsets) {
	guint32 offset = offsets[nsen];
	x = crp->words + offset;
    } else {
	x = crp->words + crp->readptr;
    }
    for(i=0; x[i].word; i++) {
	if (show_ids) {
	    printf("%d ",x[i].word);
	} else {
	    char *word = word_list_get_by_id(W, x[i].word);
	    if (x[i].flags & 0x1) {
		word = uppercase_dup(word);
	    } else if (x[i].flags & 0x2) {
		word = capital_dup(word);
	    } else {
		word = g_strdup(word);
	    }
	    printf("%s ", word);
	    free(word);
	}
    }
    printf("\n");
}

static gboolean match(CorpusCell *crp_sentence, int crp_size,
		      guint32 *ids_sentence, int ids_size) {
    int i,j;
    
    if (ids_size > crp_size) return FALSE;
    
    for (i = 0; i < crp_size - ids_size; i++) {
	for (j = 0; j<ids_size && crp_sentence[i+j].word==ids_sentence[j]; j++);
	if (j == ids_size) return TRUE;
    }
    return FALSE;
}

static guint32* load_offsets(char *filename, guint32* size) {
    gchar *file = g_strdup_printf("%s.index", filename);    
    guint32 x;
    guint32 i = 0;
    guint32 *bf;
    FILE *fd = fopen(file, "r");
    g_free(file);
    if (!fd) return NULL;

    fread(&x, sizeof(guint32), 1, fd);
    if (size) *size = x;
    bf = g_new(guint32, x);
    while(x--) fread(&bf[i++], sizeof(guint32), 1, fd);
    fclose(fd);
    return bf;
}

static double* load_ranks(guint32 x, char *filename) {
    guint32 i = 0;
    double *bf;
    FILE *fd = fopen(filename, "r");
    if (!fd) return NULL;

    bf = g_new(double, x);
    while(x--) fread(&bf[i++], sizeof(double), 1, fd);
    fclose(fd);
    return bf;
}

/**
 * @brief The main function 
 *
 * @todo Document this
 */
int main(int argc, char *argv[]) {
    char *sentence[MAXBUF], *phrase;
    guint32 size, *sizes = NULL;
    gboolean show_ranking = 0, show_ids = 0;
    guint32 snr = 0;
    guint32 ids[MAXBUF];
    CorpusCell *s, *t;
    char *rankfile = NULL;
    double *rank = NULL, **ranks = NULL;
    int s_len, n_sen, i, nsen;

    WordList *word_list_src = NULL, *word_list = NULL;
    WordLstNode **W, **O;

    Corpus *corpusA = NULL, *corpusB = NULL;
    guint32 *offset_DESTS = NULL, *offset_ORIGS = NULL;

    Corpus **AA = NULL, **BB = NULL;
    guint32 **offset_AA = NULL, **offset_BB = NULL;
    
    extern char *optarg;
    extern int optind;

    int c;
    int chunk_number = 0;

    /* code */

    while ((c = getopt(argc, argv, "c:iq:V")) != EOF) {
	switch (c) {
	case 'c':	    /* chunks */
	    chunk_number = atoi(optarg);
	    if (chunk_number < 2) {
		fprintf(stderr, "Wrong number of chunks (must be >=2)\n");
		exit(1);
	    } else
	    break;
	case 'i':
	    show_ids = TRUE;
	    break;
	case 'V':
	    printf(PACKAGE " version " VERSION "\n");
	    exit(0);
	case 'q':
	    show_ranking = TRUE;
	    rankfile = optarg;
	    break;
	case '?':
	    exit(0);
	default:
	    fprintf(stderr, "Unknown option -%c\n", c);
	    exit(1);
	}
    }

    
    W = NULL;

    if (argc != 5 + optind && argc != 4 + optind) {
	printf("Syntax:\n\t");
	printf("nat-css [-q <rankfile>] <lexicon1> <corpus1> <lexicon2> <corpus2> [<sentence_nr> | all]\n");
	return 1;
    }

    /* Load first lexicon  */
    word_list_src = word_list_load(argv[0 + optind], &O);
    if (!word_list_src) { printf("Can't open file %s\n", argv[0 + optind]); return 1; }

    /* Load first corpus... */
    if (chunk_number) {
	char *filename;
	int   iterator;

	AA        = g_new0(Corpus* , chunk_number);
	offset_AA = g_new0(guint32*, chunk_number);

	for (iterator = 0; iterator < chunk_number; ++iterator) {
	    asprintf(&filename, "%s.%d.crp", argv[1 + optind], iterator);
	    AA[iterator] = corpus_new();
	    if (corpus_load(AA[iterator], filename)) {
		printf("Can't open file %s\n", filename); return 1;
	    }
	    offset_AA[iterator] = load_offsets(filename, NULL);
	    if (!offset_AA[iterator]) {
		printf("Error opening offset files... bailing out\n"); return 1;
	    }
	    free(filename);
	}
    } else {
	corpusA = corpus_new();
	if (corpus_load(corpusA, argv[1 + optind])) {
	    printf("Can't open file %s\n", argv[1 + optind]); 
	    return 1; 
	}
	offset_ORIGS = load_offsets(argv[1 + optind], NULL);
	if (!offset_ORIGS) { printf("Error opening offset files... bailing out\n"); return 1; }
    }

    /* Load target lexicon */
    word_list = word_list_load(argv[2 + optind], &W);
    if (!word_list) { printf("Error opening target language lexicon file\n"); return 1; }

    /* Load second/target corpus... */
    if (chunk_number) {
	char *filename;
	int   iterator;

	BB        = g_new0(Corpus*,  chunk_number);
	offset_BB = g_new0(guint32*, chunk_number);
	sizes     = g_new0(guint32,  chunk_number);

	for (iterator = 0; iterator < chunk_number; ++iterator) {
	    asprintf(&filename, "%s.%d.crp", argv[1 + optind], iterator);
	    BB[iterator] = corpus_new();
	    if (corpus_load(BB[iterator], filename)) {
		printf("Can't open file %s\n", filename); return 1;
	    }
	    offset_BB[iterator] = load_offsets(filename, &sizes[iterator]);
	    if (!offset_BB[iterator]) {
		printf("Error opening offset files... bailing out\n"); return 1;
	    }
	    free(filename);
	}
    } else {
	corpusB = corpus_new();
	if (corpus_load(corpusB, argv[3 + optind])) {
	    printf("Can't open file %s\n", argv[3 + optind]); 
	    return 1; 
	}
	offset_DESTS = load_offsets(argv[3 + optind], &size);
	if (!offset_DESTS) { printf("Error opening offset files... bailing out\n"); return 1; }
    }

    /* load rankings... */
    if (show_ranking) {
	if (chunk_number) {
	    int iterator;
	    char *filename;
	    ranks = g_new(double*, chunk_number);
	    for (iterator = 0; iterator < chunk_number; ++iterator) {
		asprintf(&filename,"%s.$d",rankfile,iterator);
		ranks[iterator] = load_ranks(size, filename);
		g_free(filename);
	    }
	} else {
	    rank = load_ranks(size, rankfile);
	    if (!rank) { printf("Error opening ranking file: %s\n", rankfile); return 1; }
	}
    }

    /* ------------------------------- */
    /* Now, do the process you need... */
    /* ------------------------------- */

    if (argc == 4 + optind) {
	phrase = g_new(char, MAXBUF*5);
	while(!feof(stdin)) {
	    char *p;
	    fputs("-*- READY -*-\n", stdout);
	    fflush(stdout);
	    fgets(phrase, MAXBUF*5, stdin);
	    
	    if (feof(stdin)) break;
	    phrase[strlen(phrase)-1] = '\0';
	    p = phrase; 		/* we need this because p is re-defined, sometimes! */
	    
	    nsen = NextTextSentence(sentence, &p, MAXBUF, SOFTDELIMITER, HARDDELIMITER);
	    for (i=0; i<nsen; i++) ids[i] = word_list_get_id(word_list_src, sentence[i]);

	    if (chunk_number) {
		int iterator;
		for (iterator = 0; iterator < chunk_number; iterator++) {
		    n_sen = 0;
		    s = corpus_first_sentence(AA[iterator]);
		    s_len = corpus_sentence_length(s);
	    
		    if (match(s, s_len, ids, nsen)) {
			if (show_ranking) printf("%f\n", ranks[iterator][n_sen]);
			print_sentence(AA[iterator], O, NULL, 0, show_ids);
			print_sentence(BB[iterator], W, offset_BB[iterator], n_sen, show_ids);
		    }
	
		    while((s = corpus_next_sentence(AA[iterator]))) {
			n_sen++;
			s_len = corpus_sentence_length(s);
			
			if (match(s, s_len, ids, nsen)) {
			    if (show_ranking) printf("%f\n", ranks[iterator][n_sen]);
			    print_sentence(AA[iterator], O, NULL, 0, show_ids);
			    print_sentence(BB[iterator], W, offset_BB[iterator], n_sen, show_ids);
			}
		    }
		}
	    } else {
		n_sen = 0;
		s = corpus_first_sentence(corpusA);
		s_len = corpus_sentence_length(s);
		
		if (match(s, s_len, ids, nsen)) {
		    if (show_ranking) printf("%f\n", rank[n_sen]);
		    print_sentence(corpusA, O, NULL, 0, show_ids);
		    print_sentence(corpusB, W, offset_DESTS, n_sen, show_ids);
		}
	
		while((s = corpus_next_sentence(corpusA))) {
		    n_sen++;
		    s_len = corpus_sentence_length(s);
		    
		    if (match(s, s_len, ids, nsen)) {
			if (show_ranking) printf("%f\n", rank[n_sen]);
			print_sentence(corpusA, O, NULL, 0, show_ids);
			print_sentence(corpusB, W, offset_DESTS, n_sen, show_ids);
		    }
		}
	    }
	}
	
    }

    if (argc == 5 + optind) {
	int i = 0;
	if (strcmp(argv[4 + optind],"all") == 0) {
	    if (chunk_number) {
		int iterator;
		for (iterator = 0; iterator < chunk_number; iterator++) {
		    s = corpus_first_sentence(AA[iterator]);
		    t = corpus_first_sentence(BB[iterator]);

		    if (show_ranking) printf("%f\n", ranks[iterator][i++]);
		    print_sentence(AA[iterator], O, NULL, 0, show_ids);
		    print_sentence(BB[iterator], W, NULL, 0, show_ids);

		    while((s = corpus_next_sentence(AA[iterator])) &&
			  (t = corpus_next_sentence(BB[iterator]))) {
			if (show_ranking) printf("%f\n", ranks[iterator][i++]);
			print_sentence(AA[iterator], O, NULL, 0, show_ids);
			print_sentence(BB[iterator], W, NULL, 0, show_ids);
		    }
		}
	    } else {
		s = corpus_first_sentence(corpusA);
		t = corpus_first_sentence(corpusB);

		if (show_ranking) printf("%f\n", rank[i++]);	    
		print_sentence(corpusA, O, NULL, 0, show_ids);
		print_sentence(corpusB, W, NULL, 0, show_ids);

		while((s = corpus_next_sentence(corpusA)) &&
		      (t = corpus_next_sentence(corpusB))) {
		    if (show_ranking) printf("%f\n", rank[i++]);	    
		    print_sentence(corpusA, O, NULL, 0, show_ids);
		    print_sentence(corpusB, W, NULL, 0, show_ids);
		}
	    }
	} else {
	    snr = atol(argv[4 + optind]);
	    print_sentence(corpusA, O, offset_ORIGS, snr, show_ids);
	    print_sentence(corpusB, W, offset_DESTS, snr, show_ids);
	}
    }
    word_list_free(word_list_src);

    if (AA) g_free(AA);
    if (BB) g_free(BB);
    if (offset_AA) g_free(offset_AA);
    if (offset_BB) g_free(offset_BB);
    if (sizes) g_free(sizes);
    if (ranks) g_free(ranks);
    
    return 0;
}

