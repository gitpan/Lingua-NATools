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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <glib.h>

#include "standard.h"
#include "words.h"
#include "invindex.h"
#include "corpus.h"
#include "isolatin.h"
#include "config.h"
#include "partials.h"
#include "ngramidx.h"

/**
 * @file
 * @brief Corpora grams to database
 */

#define HASBIGRAM(x)    (x[0] && x[1]         )
#define HASTRIGRAM(x)   (HASBIGRAM(x) && x[2] )
#define HASTETRAGRAM(x) (HASTRIGRAM(x) && x[3])
#define ADDWORD(x, w)   x[0]=x[1]; x[1]=x[2]; x[2]=x[3]; x[3]=w
#define BIGRAM(x)       x[0], x[1]
#define TRIGRAM(x)      BIGRAM(x), x[2]
#define TETRAGRAM(x)    TRIGRAM(x), x[3]

static gboolean quiet;

/**
 * The main program.
 * 
 * @todo Document this
 */
int main(int argc, char **argv)
{
    Corpus *Corpus = NULL;
    SQLite *idx = NULL;
    gboolean verbose = FALSE;
    CorpusCell *cell;
    int size, i, n = -1;        /* -1 means all (2, 3 and 4) */

    extern char *optarg;
    extern int optind;
    int c;

    guint32 sentences = 0, bigrams = 0, trigrams = 0, tetragrams = 0;
    guint32 words[4] = {0, 0, 0, 0};

    setlocale(LC_CTYPE,"pt_PT.ISO8859-1");

    quiet = FALSE;
    while ((c = getopt(argc, argv, "n:jviqV")) != EOF) {
        switch (c) {
        case 'n':
            n = atoi(optarg);
            if (n > 4 || n < 2) {
                fprintf(stderr, "Ngrams value must be 2, 3 or 4, not '%d'.\n", n);
                exit(0);
            }
            break;
        case 'V':
            printf(PACKAGE " version " VERSION "\n");
            exit(1);
        case 'v':
            verbose = TRUE;
            break;
        case 'q':
            quiet = TRUE;
            break;
        default:
            fprintf(stderr, "Unknown option -%c\n", c);
            exit(1);
        }
    }

    if (quiet && verbose) {
        fprintf(stderr, "Quiet and verbose can't work together\n");
        return 1;
    }

    if ((argc != 2 + optind)) {
        printf("%s: wrong number of arguments\n", argv[0]);
        printf("\tUsage: nat-ngrams <crp-file> <sqlite-db>\n");
        exit(1);
    }

    /**
     * Load corpus
     */
    Corpus = corpus_new();
    corpus_load(Corpus, argv[optind + 0]);

    idx = ngram_index_new(argv[optind + 1], n); 

    if (!idx) {
        printf("Error creating ngrams sqlite file '%s'\n",argv[optind+1]);
        exit(1);
    }

    /**
     * Dump the crp to DB
     */
    sentences = bigrams = trigrams = tetragrams = 0;
    printf("\n Sentences\tBi\tTri\tTetra\n");
    cell = corpus_first_sentence(Corpus);       
    do {
        sentences++;
        size = corpus_sentence_length(cell);
            
        for(i=0; i<size; i++) {
            ADDWORD(words, cell[i].word);
            if(HASBIGRAM(words)) {
                /* Note, this can't be 'ANDED' in the previous if */
                if (n==2 || n==-1) {
                    bigrams++;
                    bigram_add_occurrence(idx, BIGRAM(words));
                }
            }
            else continue;
            
            if(HASTRIGRAM(words)) {
                /* Note, this can't be 'ANDED' in the previous if */
                if (n==3||n==-1)   {
                    trigrams++;
                    trigram_add_occurrence(idx, TRIGRAM(words));
                }
            }
            else continue;
            
            if(HASTETRAGRAM(words)) {
                /* Note, this can't be 'ANDED' in the previous if */
                if (n==4||n==-1) {
                    tetragrams++;
                    tetragram_add_occurrence(idx, TETRAGRAM(words));
                }
            }
        }   
        
        while(HASBIGRAM(words) && (n==2||n==-1)) {
            bigrams++;
            bigram_add_occurrence(idx, BIGRAM(words));
            
            if(HASTRIGRAM(words) && (n==3||n==-1))   {
                trigrams++;
                trigram_add_occurrence(idx, TRIGRAM(words));
            }
            if(HASTETRAGRAM(words) && (n==4||n==-1)) {
                tetragrams++;
                tetragram_add_occurrence(idx, TETRAGRAM(words));
            }
            ADDWORD(words, 0);
        }
	
        if(!(sentences%100)) { 
            printf(" %u\t\t%u\t%u\t%u\r", sentences, bigrams, trigrams, tetragrams); 
        }
    }
    while((cell = corpus_next_sentence(Corpus)));

    if(verbose) printf("\nClosing the database now\n");
    ngram_index_close(idx);

    if(verbose) printf("Database closed\n");
    corpus_free(Corpus);
    printf(" %u\t\t%u\t%u\t%u\r", sentences, bigrams, trigrams, tetragrams); 
    printf("\n\n");
    
    return 0;
}

