/* -*- Mode: C -*- */

/* NATools - Package with parallel corpora tools
 * Copyright (C) 2002-2011  Alberto Simões
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

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "dictionary.c"
#include "words.c"
#include "natdict.c"
#include "natlexicon.c"
#include "corpus.c"
#include "invindex.c"
#include "parseini.c"
#include "corpusinfo.c"
#include "bucket.c"
#include "standard.c"
#include "srvshared.c"
#include "ngramidx.c"


#define MAXDICS 10

NATDict      *natdics[MAXDICS];

Dictionary   *dics[MAXDICS];

WordList     *wls[MAXDICS];
WordLstNode **wln[MAXDICS];

Corpus       *corpus[MAXDICS];

CorpusInfo   *crp;

int first_empty_dic = 0;
int first_empty_natdic = 0;
int first_empty_wl = 0;
int first_empty_corpus = 0;
int inited = 0;

void init(void) {
    int i;
    for (i = 0; i < MAXDICS; i++) {
	dics[i] = NULL;
	wls[i] = NULL;
	wln[i] = NULL;
	natdics[i] = NULL;
	corpus[i] = NULL;
    }
    crp = NULL;
    inited = 1;
}

MODULE = Lingua::NATools		PACKAGE = Lingua::NATools



U32
dicgetsize(id)
         int id
   INIT:
         U32 size;
   CODE:
         if (id > MAXDICS || id < 0 || !dics[id]) {
	     size = -1;
         } else {
	     size = dictionary_get_size(dics[id]);
	 }
         RETVAL = size;
   OUTPUT:
         RETVAL



U32
dicgetocc(id, word)
         int id
         U32 word
   INIT:
         U32 occ;
   CODE:
         if (id > MAXDICS || id < 0 || !dics[id]) {
	     occ = 0;
         } else {
	     occ = dictionary_get_occ(dics[id], word);
	 }
         RETVAL = occ;
   OUTPUT:
         RETVAL


int
dicenlarge(id, nsize)
         int id
         U32 nsize
   INIT:
         int retval;
   CODE:
         if (id > MAXDICS || id < 0 || !dics[id]) {
	     retval = -1;
         } else {
	     dictionary_realloc(dics[id], nsize);
	     retval = 0;
	 }
         RETVAL = retval;
   OUTPUT:
         RETVAL

       


int
dicsetocc(id, word, occ)
         int id
         U32 word
	 U32 occ
   INIT:
         int retval;
   CODE:
         if (id > MAXDICS || id < 0 || !dics[id]) {
	     retval = -1;
         } else {
	     dictionary_set_occ(dics[id], word, occ);
	     retval = 0;
	 }
         RETVAL = retval;
   OUTPUT:
         RETVAL


int
dicsetvals(id, wid, offset, twid, val)
         int id
         U32 wid
         U32 offset
         U32 twid
         float val
   INIT:
         int retval = 0;
   CODE:
         if (id > MAXDICS || id < 0 || !dics[id]) {
	     retval = -1;
         } else {
             dictionary_set_id(dics[id], wid, offset, twid);
	     dictionary_set_val(dics[id], wid, offset, val);
	     retval = 0;
	 }
         RETVAL = retval;
   OUTPUT:
         RETVAL




SV*
dicgetvals(id, word)
         int id
         U32 word
   INIT:
         AV* array;
   CODE:
         if (id > MAXDICS || id < 0 || !dics[id]) {
	     XSRETURN_UNDEF;
         } else {
	     int i;
	     guint32 wid;
	     float val;
	     array = newAV();
	     for (i=0;i<MAXENTRY;i++) {
		 wid = dictionary_get_id(dics[id],word,i);
		 val = dictionary_get_val(dics[id],word,i);
		 if (val == 0.0) break;
		 av_push(array, newSVuv(wid));
		 av_push(array, newSVnv(val));
	     }
	 }
         RETVAL = newRV_noinc((SV*)array);
   OUTPUT:
         RETVAL

void
dicclose(id)
         int id
   CODE:
         if (id > MAXDICS || id < 0) return;
         if (dics[id]) dictionary_free(dics[id]);
         dics[id] = NULL;
         if (id < first_empty_dic) first_empty_dic = id;


int
dicsave(id, filename)
        int id
        char *filename
   CODE:
        if (id > MAXDICS || id < 0 || !dics[id]) {
            RETVAL = 0;
	} else {
	    RETVAL = dictionary_save(dics[id], filename);
	}
   OUTPUT:
        RETVAL


int
dicadd(id1, id2)
        int id1
        int id2
   INIT:
        int nid;
   CODE:
        if (id1 > MAXDICS || id1 < 0 || !dics[id1] ||
	    id2 > MAXDICS || id2 < 0 || !dics[id2] ||
	    first_empty_dic == MAXDICS) {
	    nid = -1;
	} else {
	    nid = first_empty_dic;
	    dics[nid] = dictionary_add(dics[id1], dics[id2]);
            while(first_empty_dic < MAXDICS) {
		if (!dics[first_empty_dic]) break;
		first_empty_dic++;
	    }
	}
        RETVAL = nid;
   OUTPUT:
        RETVAL


int
dicnew(size)
        U32 size
   INIT:
        int i = first_empty_dic;
   CODE:
        if (!inited) init();
        if (first_empty_dic == MAXDICS) {
	    croak("Maximum number of dictionaries opened");
	    RETVAL = -1;
        } else {
            dics[first_empty_dic] = dictionary_new(size);
            while(first_empty_dic < MAXDICS) {
                if (!dics[first_empty_dic]) break;
                first_empty_dic++;
            }
            RETVAL = i;
   	}
   OUTPUT:
        RETVAL



int
dicopen(filename)
        char* filename
   INIT:
        int i = first_empty_dic;
   CODE:
        if (!inited) init();
        if (first_empty_dic == MAXDICS) {
	    croak("Maximum number of dictionaries opened");
	    RETVAL = -1;
        } else {
            dics[first_empty_dic] = dictionary_open(filename);
            while(first_empty_dic < MAXDICS) {
              if (!dics[first_empty_dic]) break;
              first_empty_dic++;
	    }
	    RETVAL = i;
   	}

   OUTPUT:
        RETVAL


double
fulldicsim(dic1, dic2, s1, s2)
        int dic1
        int dic2
        AV* s1
        AV* s2
   CODE:
        {
	    double tmp;
	    if (dic1 > MAXDICS || dic1 < 0 || !dics[dic1] ||
		dic2 > MAXDICS || dic2 < 0 || !dics[dic2]) {
		RETVAL = 0;
	    } else {
		U32 s1size, s2size, i;
		unsigned int *ss1, *ss2;
		
		s1size = av_len(s1)+1;
		s2size = av_len(s2)+1;
		ss1 = (unsigned int*)malloc(sizeof(int)*s1size);
		ss2 = (unsigned int*)malloc(sizeof(int)*s2size);
		for(i=0;i<s1size;i++) {
		    SV **sv = av_fetch(s1,i,1);
		    ss1[i] = SvNV(*sv);
		}
		for(i=0;i<s2size;i++) {
		    SV **sv = av_fetch(s2,i,1);
		    ss2[i] = SvNV(*sv);
		}
		tmp = dictionary_sentence_similarity(dics[dic1],
						     ss1, s1size,
						     ss2, s2size);
		tmp += dictionary_sentence_similarity(dics[dic2],
						      ss2, s2size,
						      ss1, s1size);
		RETVAL = tmp/2;
		free(ss1);
		free(ss2);
	    }
	}
    OUTPUT:
	RETVAL




int
wlopen(filename)
        char* filename
   INIT:
        int i = first_empty_wl;
   CODE:
        if (!inited) init();
        if (first_empty_wl == MAXDICS) {
	    croak("Maximum number of wordlist opened");
	    RETVAL = -1;
        } else {
            wls[first_empty_wl] = word_list_load(filename, &wln[first_empty_wl]);
            while(first_empty_wl < MAXDICS) {
              if (!wls[first_empty_wl]) break;
              first_empty_wl++;
	    }
	RETVAL = i;
   	}

   OUTPUT:
        RETVAL


void
wlclose(id)
         int id
   CODE:
         if (id > MAXDICS || id < 0) return;
         if (wls[id]) word_list_free(wls[id]);
         free(wln[id]);
         wls[id] = NULL;
         wln[id] = NULL;
         if (id < first_empty_wl) first_empty_wl = id;


char*
wlgetbyid(id, word)
         int id
         U32 word
   INIT:
         char* str;
   CODE:
         if (id > MAXDICS || id < 0 || !wln[id]) {
	     str = g_strdup("");
         } else {
	     str = word_list_get_by_id(wln[id], word);
	     if (!str) str = g_strdup("(none)");
	 }
         RETVAL = str;
          /*   CLEANUP: free(str);  */ /* or similar call to free the memory */
   OUTPUT:
         RETVAL


int
wlenlarge(id, extracells)
         int id
         U32 extracells
    INIT:    
	 int retval;
    CODE:
         if (id > MAXDICS || id < 0 || !wln[id]) {
	     retval = 1;
         } else {
	     wln[id] = word_list_enlarge(wls[id], wln[id], extracells);
	     retval = 0;
	 }
         RETVAL = retval;
    OUTPUT:
         RETVAL

U32
wladdword(id, word)
         int id
         char *word
   INIT:
         U32 wid;
   CODE:
         if (id > MAXDICS || id < 0 || !wln[id]) {
	     wid = 0;
         } else {
	     wid = word_list_add_word_and_index(wls[id], wln[id], g_strdup(word));
	 }
         RETVAL = wid;
   OUTPUT:
         RETVAL
 

U32
wlgetbyword(id, word)
         int id
         char *word
   INIT:
         U32 wid;
   CODE:
         if (id > MAXDICS || id < 0 || !wln[id]) {
	     wid = 0;
         } else {
	     wid = word_list_get_id(wls[id], word);
	 }
         RETVAL = wid;
   OUTPUT:
         RETVAL

int
wlsave(id, filename)
         int id
         char* filename
   INIT:
         int retval;
   CODE:
         retval = 1;
         if (id > MAXDICS || id < 0 || !wln[id]) {
             retval = 0;
         } else {
	     word_list_save(wls[id], filename);
         }
         RETVAL = retval;
   OUTPUT:
         RETVAL


U32
wlgetsize(id)
         int id
   INIT:
         U32 size;
   CODE:
         if (id > MAXDICS || id < 0 || !wln[id]) {
	     size = 0;
         } else {
	     size = word_list_size(wls[id]);
	 }
         RETVAL = size;
   OUTPUT:
         RETVAL


U32
wloccs(id)
         int id
   INIT:
         U32 size;
   CODE:
         if (id > MAXDICS || id < 0 || !wln[id]) {
	     size = 0;
         } else {
	     size = word_list_occurrences(wls[id]);
	 }
         RETVAL = size;
   OUTPUT:
         RETVAL


U32
wlcountbyid(id, wid)
         int id
	 U32 wid
   INIT:
         U32 size;
   CODE:
         if (id > MAXDICS || id < 0 || !wln[id]) {
	     size = 0;
         } else {
	     size = word_list_get_count_by_id(wln[id], wid);
	 }
         RETVAL = size;
   OUTPUT:
         RETVAL

int
wlsetcountbyid(id, wid, count)
         int id
	 U32 wid
         U32 count
   INIT:
         U32 retval;
   CODE:
         if (id > MAXDICS || id < 0 || !wln[id]) {
	     retval = 0;
         } else {
	     retval = word_list_set_count_by_id(wls[id], wln[id], wid, count);
	 }
         RETVAL = retval;
   OUTPUT:
         RETVAL



int
nat_dict_open(filename)
        char* filename
   INIT:
        int i = first_empty_natdic;
   CODE:
        if (!inited) init();
        if (first_empty_natdic == MAXDICS) {
	    croak("Maximum number of NATDicts opened");
	    RETVAL = -1;
        } else {
            natdics[first_empty_natdic] = natdict_open(filename);
            while(first_empty_natdic < MAXDICS) {
              if (!natdics[first_empty_natdic]) break;
              first_empty_natdic++;
	    }
	RETVAL = i;
   	}

   OUTPUT:
        RETVAL

void
nat_dict_close(id)
         int id
   CODE:
         if (id > MAXDICS || id < 0) return;
         if (natdics[id]) natdict_free(natdics[id]);
         natdics[id] = NULL;
         if (id < first_empty_natdic) first_empty_natdic = id;


char*
nat_dict_source_lang(id)
         int id
   CODE:
         if (id > MAXDICS || id < 0) RETVAL = NULL;
         else if (!natdics[id]) RETVAL = NULL;
         else RETVAL = natdics[id]->source_language;
   OUTPUT:
         RETVAL


char*
nat_dict_target_lang(id)
         int id
   CODE:
         if (id > MAXDICS || id < 0) RETVAL = NULL;
         else if (!natdics[id]) RETVAL = NULL;
         else RETVAL = natdics[id]->target_language;
   OUTPUT:
         RETVAL


char*
nat_dict_word_from_id(id, lang, wid)
        int id
        int lang
        U32 wid
   CODE:
         if (id > MAXDICS || id < 0) RETVAL = NULL;
         else if (!natdics[id]) RETVAL = NULL;
         else RETVAL = natdict_word_from_id(natdics[id],lang?1:0,wid);
   OUTPUT:
        RETVAL


U32
nat_dict_id_from_word(id, lang, wid)
        int id
        int lang
        char* wid
   CODE:
         if (id > MAXDICS || id < 0) RETVAL = 0;
         else if (!natdics[id]) RETVAL = 0;
         else RETVAL = natdict_id_from_word(natdics[id],lang?1:0,wid);
   OUTPUT:
        RETVAL


int
nat_dict_add_files(f1, f2, f3)
        char* f1
        char* f2
        char* f3
   CODE:
        RETVAL = 1;
	NATDict *d1 = natdict_open(f1);
        if (!d1) {
	    RETVAL = 0;
        } else {
	    NATDict *d2 = natdict_open(f2);
	    if (!d2) {
		RETVAL = 0;
	    } else {
		NATDict *d3 = natdict_add(d1, d2);
		if (!d3) {
		    RETVAL = 0;
		} else {
		    natdict_save(d3, f3);
		}
	    }
	}
   OUTPUT:
         RETVAL



U32
nat_dict_word_count(id, lang, wid)
        int id
        int lang
        U32 wid
   CODE:
         if (id > MAXDICS || id < 0) RETVAL = 0;
         else if (!natdics[id]) RETVAL = 0;
         else RETVAL = natdict_word_count(natdics[id], lang?1:0, wid);
   OUTPUT:
        RETVAL

SV*
nat_dict_getvals(id, lang, word)
         int id
         int lang
         U32 word
   INIT:
         AV* array;
   CODE:
         if (id > MAXDICS || id < 0 || !natdics[id]) {
	     XSRETURN_UNDEF;
         } else {
	     guint32 i;
	     guint32 wid;
	     float val;
	     array = newAV();
	     for (i=0;i<MAXENTRY;i++) {
		 wid = natdict_dictionary_get_id(natdics[id],lang,word,i);
		 val = natdict_dictionary_get_val(natdics[id],lang,word,i);
		 if (val == 0.0) break;
		 av_push(array, newSVuv(wid));
		 av_push(array, newSVnv(val));
	     }
	 }
         RETVAL = newRV_noinc((SV*)array);
   OUTPUT:
         RETVAL




int
corpus_open(filename)
        char* filename
   INIT:
        int i = first_empty_corpus;
   CODE:
        if (!inited) init();
        if (first_empty_corpus == MAXDICS) {
	    croak("Maximum number of corpora files opened");
	    RETVAL = -1;
        } else {
            corpus[first_empty_corpus] = corpus_new();
	    if (corpus_load(corpus[first_empty_corpus], filename)) {
		corpus_free(corpus[first_empty_corpus]);
		RETVAL = -1;
	    } else {
		while(first_empty_corpus < MAXDICS) {
		    if (!corpus[first_empty_corpus]) break;
		    first_empty_corpus++;
		}
		RETVAL = i;
	    }
   	}
   OUTPUT:
        RETVAL


U32
corpus_sentences_nr_xs(id)
         int id
    INIT:
         U32 size;
    CODE:
         if (id > MAXDICS || id < 0 || !corpus[id]) {
	     size = 0;
         } else {
	     size = corpus_sentences_nr(corpus[id]);
	 }
         RETVAL = size;
     OUTPUT:
         RETVAL


SV*
corpus_first_sentence_xs(id)
        int id
   INIT:
        AV* array;
   CODE:
        if (id > MAXDICS || id < 0 || !corpus[id]) {
	    XSRETURN_UNDEF;
	} else {
	    guint32 wid;
	    CorpusCell *w;
	    
	    array = newAV();
	    w = corpus_first_sentence(corpus[id]);
	    while((wid = w->word)) {
		av_push(array, newSVuv(wid));
		w++;
	    }
	}
        RETVAL = newRV_noinc((SV*)array);
   OUTPUT:
        RETVAL



SV*
corpus_next_sentence_xs(id)
        int id
   INIT:
        AV* array;
   CODE:
        if (id > MAXDICS || id < 0 || !corpus[id]) {
	    XSRETURN_UNDEF;
	} else {
	    guint32 wid;
	    CorpusCell *w;

	    array = newAV();
	    w = corpus_next_sentence(corpus[id]);
	    if (w == NULL) XSRETURN_UNDEF;
	    while((wid = w->word)) {
		av_push(array, newSVuv(wid));
		w++;
	    }
	}
        RETVAL = newRV_noinc((SV*)array);
   OUTPUT:
        RETVAL


void
corpus_free_xs(id)
        int id
   CODE:
        if (id > MAXDICS || id < 0 || !corpus[id]) {
	    XSRETURN_UNDEF;
	} else {
	    corpus_free(corpus[id]);
	    corpus[id] = NULL;
            if (id < first_empty_corpus) first_empty_corpus = id;
	}








void
corpus_info_open(filename)
        char* filename
   INIT:

   CODE:
        if (crp) corpus_info_free(crp);
        crp = corpus_info_new(filename);


void
corpus_info_free()
    CODE:
        if (crp) corpus_info_free(crp);

U32
corpus_info_lexicon_size(dir)
        int dir
    INIT:
	WordList *lexicon;
    CODE:
	if (!crp) {
	    XSRETURN_UNDEF;
	}
        lexicon = (dir > 0)?crp->SourceLex:crp->TargetLex;
        RETVAL = word_list_size(lexicon);
    OUTPUT:
        RETVAL


char*
corpus_info_word_from_wid(dir, wid)
        int dir
	U32 wid
    INIT:
	WordLstNode **lexicon;
	char *w;
    CODE:
	if (!crp) {
	    XSRETURN_UNDEF;
	}
        lexicon = (dir > 0)?crp->SourceLexIds:crp->TargetLexIds;
        w = word_list_get_by_id(lexicon, wid);
        RETVAL = w;
    OUTPUT:
        RETVAL


SV*
corpus_info_ptd_by_word(dir, word)
        int dir
        char *word
    INIT:
        WordList *S;
        WordLstNode **T;
        Dictionary *D;
        guint32 wid, twid;
	AV* array;
	float prob;
        int j;
	char *tword;
    CODE:
        if (!crp) {
	    XSRETURN_UNDEF;
	}
        if (dir > 0) {
	    S = crp->SourceLex;
	    T = crp->TargetLexIds;
	    D = crp->SourceTarget;
	} else {
	    S = crp->TargetLex;
	    T = crp->SourceLexIds;
	    D = crp->TargetSource;
	}

        wid = word_list_get_id(S, word);
        if (!wid) {
	    XSRETURN_UNDEF;
	}

        array = newAV();
        av_push(array, newSVuv(dictionary_get_occ(D, wid)));

        for (j = 0; j < MAXENTRY; j++) {
	    twid = 0;
	    prob = 0.0;
	    twid = dictionary_get_id(D, wid, j);
	    if (twid) {
		prob = dictionary_get_val(D, wid, j);
		tword = word_list_get_by_id(T, twid);
                if (!tword) { fprintf(stderr, "Id: %d\n", twid); }
		av_push(array, newSVpvn(tword, strlen(tword)));
		av_push(array, newSVnv((double)prob));
	    }
	}

        RETVAL = newRV_noinc((SV*)array);
    OUTPUT:
        RETVAL



SV*
corpus_info_conc_by_str(direction, both, exact_match, words_str)
        int direction
        int both
        int exact_match
        char *words_str
    INIT:
	int i;
	AV* array;
	AV* triple;
	char words[50][150];
        char *token = NULL;
        GSList *list, *iterator = NULL;
    CODE:
        if (!crp) {
	    XSRETURN_UNDEF;
	}

        for (i=0;i<50;i++) strcpy(words[i],"");
        i = 0;

        token = strtok(words_str, " ");
        while(token) {
	    strcpy(words[i], token);
	    i++;
	    token = strtok(NULL, " ");
	}

        list = dump_conc(0, crp, direction, both, exact_match, words, i);

        array = newAV();
        for (iterator = list; iterator; iterator = iterator->next) {
	    TU *tu = (TU*)iterator->data;
	    triple = newAV();
	    
	    if (tu->quality >= 0.0) av_push(triple, newSVnv(tu->quality));
	    av_push(triple, newSVpvn(tu->source, strlen(tu->source)));
	    av_push(triple, newSVpvn(tu->target, strlen(tu->target)));
	    av_push(array, newRV_noinc((SV*)triple));
	    destroy_TU(tu);
	}
        g_slist_free(list);

        RETVAL = newRV_noinc((SV*)array);
    OUTPUT:
        RETVAL


SV*
corpus_info_ngrams_by_str(direction, query)
        int direction
        char *query
    INIT:
        int i;
        AV* array;
        AV* gram;
	char words[50][150];
        char *token = NULL;
        GSList *list, *iterator = NULL, *siterator = NULL;
    CODE:
        if (!crp) {
            XSRETURN_UNDEF;
        }
       
        for (i=0;i<50;i++) strcpy(words[i],"");
        i = 0;

        token = strtok(query, " ");
        while(token) {
	    strcpy(words[i], token);
	    i++;
	    token = strtok(NULL, " ");
	}

        list = dump_ngrams(0, crp, direction, words, i);

        array = newAV();
        for (iterator = list; iterator; iterator = iterator->next) {
	    gram = newAV();
            siterator = (GSList*)iterator->data;
            for (; siterator; siterator = siterator->next) {
                av_push(gram, newSVpvn((char*)siterator->data, strlen((char*)siterator->data)));
                g_free(siterator->data);
            }
            av_push(array, newRV_noinc((SV*)gram));
            g_slist_free((GSList*)iterator->data);
        }
        g_slist_free(list);

        RETVAL = newRV_noinc((SV*)array);
    OUTPUT:
        RETVAL


