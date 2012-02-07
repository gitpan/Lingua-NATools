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

#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
/* #include <bits/signum.h> */
#include <time.h>
#include <stdlib.h>



#include <glib.h>

#include "parseini.h"
#include "corpusinfo.h"
#include "srvshared.h"

#define DEBUG 0

int sockfd, fd;

GHashTable *CORPORA;
gint32 LAST_CORPORA = 0;

void LOG(char* log, ...) {
    char stime[80];
    time_t timep;

    va_list args;
    va_start(args, log);

    time(&timep);
    strftime(stime, 80, "%F %T", localtime(&timep));

    fprintf(stderr, "[%s] ", stime);
    vfprintf(stderr, log, args);
    fprintf(stderr,"\n");
    va_end(args);
}

#if DEBUG
static guint32 length(const guint32 *needle) {
    guint32 ans = 0;
    while(needle[ans]) { ans++; }
    return ans;
}
#endif


void parse(int fd, char* buf, char* dir);

static int dump_corpora_list(int fd, gint last, GHashTable *corpora) {
    gboolean toexit = FALSE;
    gchar sbuf[1024];
    gint i;

    sprintf(sbuf, "%d\n", last);
    if (write(fd, sbuf, strlen(sbuf)) < 0) 
	toexit = TRUE;

    for (i = 1; i <= last && !toexit; i++) {
	sprintf(sbuf, "[%d] %s\n", 
		i,
		(char*)g_hash_table_lookup(((CorpusInfo*)g_hash_table_lookup(corpora, &i))->config,"name"));
	if (write(fd, sbuf, strlen(sbuf)) < 0) 
	    toexit = TRUE;
    }
    return toexit;
}

static int dump_conf(CorpusInfo *corpus, int fd, char *key) {
    gchar sbuf[1024];
    gboolean toexit = FALSE;

    sprintf(sbuf, "%s\n", (char*)g_hash_table_lookup(corpus->config, key));
    if (write(fd, sbuf, strlen(sbuf)) < 0) 
	toexit = TRUE;

    return toexit;
}

static void dump_each_conf(gpointer key, gpointer value, gpointer user_data) {
    gchar sbuf[1024];
    int *fd = (int*)user_data;

    sprintf(sbuf, "%s=%s\n", (char*)key, (char*)value);
    write(*fd, sbuf, strlen(sbuf));
}

static int dump_all_conf(CorpusInfo *corpus, int fd) {
    int x = fd;
    g_hash_table_foreach(corpus->config, dump_each_conf, &x);
    DONE(fd);
    return 0;
}

static CorpusInfo* get_corpus(gint id) {
    if (id > LAST_CORPORA)
	return NULL;
    return (CorpusInfo*)g_hash_table_lookup(CORPORA, &id);
}





void dump_dict_full(int fd, char *word, guint32 wid, gint32 dir, CorpusInfo *crp)
{
    char sbuf[300];
    guint32 j;
    guint32 twid;
    WordLstNode **T;
    Dictionary *D;
    float prob;

    if (strcmp(word, "")==0) {
	DONE(fd); 
	return;
    }

    if (dir > 0) {
        T = crp->TargetLexIds;
        D = crp->SourceTarget;
    } else {
        T = crp->SourceLexIds;
        D = crp->TargetSource;
    }
    
    sprintf(sbuf, "%s\n", word);
    write(fd, sbuf, strlen(sbuf));

    sprintf(sbuf, "%u\n", dictionary_get_occ(D, wid));
    write(fd, sbuf, strlen(sbuf));

    for (j = 0; j < MAXENTRY; j++) {
	twid = 0;
	prob = 0.0;
	twid = dictionary_get_id(D, wid, j);
	if (twid) {
	    prob = dictionary_get_val(D, wid, j);
	    sprintf(sbuf, "%.6f %s\n", prob, word_list_get_by_id(T, twid));
	    write(fd, sbuf, strlen(sbuf));
	}
    }
    
    DONE(fd);

    return;    
}



void dump_dict_n(int fd, char* word, gint32 dir, CorpusInfo* crp) {
    char *w;
    char sbuf[300];
    guint32 wid;

    WordLstNode **S;

    S = (dir > 0) ?  crp->SourceLexIds : crp->TargetLexIds;
    wid = atoi(word);
    w = strdup(word_list_get_by_id(S, wid));
    dump_dict_full(fd, sbuf, wid, dir, crp);
    free(w);
}

void dump_dict_w(int fd, char* word, gint32 dir, CorpusInfo* crp) {
    guint32 wid;
    WordList *S;

    if (strcmp(word, "")==0) {
	DONE(fd); 
	return;
    }

    S = (dir > 0) ? crp->SourceLex : crp->TargetLex ;
    wid = word_list_get_id(S, word);
    if (!wid) { 
	ERROR(fd);
	return;
    }

    dump_dict_full(fd, word, wid, dir, crp);
}


void handle_sigpipe(int x) {
    LOG("Connection abruptely closed");
}

void handle_sigalrm(int x) {
    LOG("Timeout, dude!");
    close(fd);
}

void handle_sigint(int x) {
    LOG("Quiting...");
    close(fd);
    close(sockfd);
    exit(0);
}

int* int_ptr(gint x) {
    gint *y;
    y = g_new(gint, 1);
    *y = x;
    return y;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serv;
    struct sockaddr_in client;
    //    struct in_addr *aux;

    short port = 4000;

    //    char *host = "193.136.19.131";

    char buf[1024];
    size_t n;
    unsigned int size = sizeof(client);
    int zbr = 1;
    FILE *f = NULL;

    if (argc != 2) {
	printf("Usage: nat-server <config-file>\n");
	return 0;
    }

    CORPORA = g_hash_table_new(g_int_hash, g_int_equal);

    /* CONFIGURATION FILE */
    LOG("Loading configuration file");
    f = fopen(argv[1], "rb");
    if (f) {
	CorpusInfo *tmp_corpus;
	while(!feof(f)) {
	    fgets(buf, 1024, f);
	    if (!feof(f)) {
		if (buf[0] == '\n' || buf[0] == '#' || buf[0] == ' ') continue;
		chomp(buf);
		LOG("Loading corpus from %s [%d]", buf, ++LAST_CORPORA);
		tmp_corpus = corpus_info_new(buf);
		g_hash_table_insert(CORPORA, int_ptr(LAST_CORPORA), tmp_corpus);
	    }
	}
	fclose(f);
    } else {
	report_error("Can't find '%s'", argv[1]);
    }
    
    /* SERVER CODE */
    /*-------------*/

    signal(SIGALRM, handle_sigalrm);
    signal(SIGINT, handle_sigint);
    signal(SIGPIPE, handle_sigpipe);

    memset(&serv, 0, sizeof(serv));

    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
 
    if (( sockfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
	LOG("couldn't create socket!");
	return -1;
    } else {
	LOG("socket created!");
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &zbr, sizeof zbr);

    if ((bind(sockfd, (struct sockaddr *) &serv, sizeof(serv))) <0 ) {
	LOG("couldn't bind!");
	return -1;
    } else {
	LOG("bind!");
    }

  
    if (listen(sockfd,5) < 0 ) {
	LOG("couldn't listen");
	return -1;
    } else{
	LOG("listen done");
    }

    for(;;) {
	if ((fd = accept(sockfd, (struct sockaddr *) &client, &size)) < 0 ) {
	    LOG("couln't accept connection!");
	    return(-1);
	} else {
#if DEBUG	    
	    LOG("accept connection (from: %s:%d) (fd: %d)",
		inet_ntoa((struct in_addr)client.sin_addr),
		ntohs(client.sin_port),
		fd); 
#endif
	    alarm(50);
	    n = read(fd, buf, 1024);
	    if ( n > 0 ) {
		parse(fd, buf, argv[1]);
		buf[0] = '\0';
		n = 0;
	    }
	    alarm(0);
	    close(fd);  
	}
    }
}

void play(int fd) {
    char buff[1000];
    strcpy(buff, "<html>\n");
    write(fd, buff, strlen(buff));
    strcpy(buff, "<body>\n");
    write(fd, buff, strlen(buff));
    strcpy(buff, "<table width='100%' height='100%'>\n");
    write(fd, buff, strlen(buff));
    strcpy(buff, "<tr><td align='center' valign='middle'>\n");
    write(fd, buff, strlen(buff));
    strcpy(buff, "<span style='color: #ff0000; padding: 20px; font-size: xx-large; border: solid 1px #ff0000'>I am logging you!</span>\n");
    write(fd, buff, strlen(buff));
    strcpy(buff, "</td></tr>\n");
    write(fd, buff, strlen(buff));
    strcpy(buff, "</table>\n");
    write(fd, buff, strlen(buff));
    strcpy(buff, "</body>\n");
    write(fd, buff, strlen(buff));
    strcpy(buff, "</html>\n");
    write(fd, buff, strlen(buff));
}


void parse(int fd, char* buf, char* dir)
{
    /* word word */

    CorpusInfo *corpus = NULL;

    int direction = 0;
    gboolean exact_match = FALSE;
    gboolean both = FALSE;
    char words[50][150];
    int i = 0; 
    char *token = NULL;
		
    chomp(buf);

    for (i=0;i<50;i++)
        strcpy(words[i],"");
    i = 0;

    if (strcmp(buf, "") == 0) return;

#if DEBUG
    LOG("Request was [%s]", buf);
#endif

    token = strtok(buf, " ");
    while(token) {
    	strcpy(words[i], token);
    	i++;
    	token = strtok(NULL, " ");
    }

    if (strncmp(words[0], "LIST", 4) == 0) {
    	dump_corpora_list(fd, LAST_CORPORA, CORPORA);
    	return;
    } else if (strncmp(words[0], "??", 2) == 0) {
        dump_all_conf(get_corpus(atoi(words[1])), fd);
        return;
    } else if (strncmp(words[0], "?", 1) == 0) {
    	dump_conf(get_corpus(atoi(words[1])), fd, words[2]);
    	return;
    } else if (strncmp(words[0], "~>", 2) == 0) {
    	corpus = get_corpus(atoi(words[1]));
    	dump_dict_w(fd, words[2], 1, corpus);
    	return;
    } else if (strncmp(words[0], "~#>", 3) == 0) {
    	corpus = get_corpus(atoi(words[1]));
    	dump_dict_n(fd, words[2], 1, corpus);
    	return;
    } else if (strncmp(words[0], "<~", 2) == 0) {
    	corpus = get_corpus(atoi(words[1]));
    	dump_dict_w(fd, words[2], -1, corpus);
    	return;
    } else if (strncmp(words[0], "<#~", 3) == 0) {
    	corpus = get_corpus(atoi(words[1]));
    	dump_dict_n(fd, words[2], -1, corpus);
    	return;
    } else if (strncmp(words[0], "<->", 3) == 0) {
    	direction = 1;
    	both = TRUE;
    	exact_match = FALSE;

    	corpus = get_corpus(atoi(words[1]));
    	dump_conc(fd, corpus, direction, both, exact_match, words, i);
    } else if (strncmp(words[0], "<=>", 3) == 0) {
    	direction = 1;
    	both = TRUE;
    	exact_match = TRUE;

    	corpus = get_corpus(atoi(words[1]));
    	dump_conc(fd, corpus, direction, both, exact_match, words, i);
    } else if (strncmp(words[0], "<-", 2) == 0) {
    	direction = -1;
    	both = FALSE;
    	exact_match = FALSE;

    	corpus = get_corpus(atoi(words[1]));
    	dump_conc(fd, corpus, direction, both, exact_match, words, i);
    } else if (strncmp(words[0], "->", 2) == 0) {
    	direction = 1;
    	both = FALSE;
    	exact_match = FALSE;

    	corpus = get_corpus(atoi(words[1]));
    	dump_conc(fd, corpus, direction, both, exact_match, words, i);
    } else if (strncmp(words[0], "<=", 2) == 0) {
    	direction = -1;
    	both = FALSE;
    	exact_match = TRUE;

    	corpus = get_corpus(atoi(words[1]));
    	dump_conc(fd, corpus, direction, both, exact_match, words, i);
    } else if (strncmp(words[0], "=>", 2) == 0) {
    	direction = 1;
    	both = FALSE;
    	exact_match = TRUE;

    	corpus = get_corpus(atoi(words[1]));
    	dump_conc(fd, corpus, direction, both, exact_match, words, i);
    } else if (strncmp(words[0], ":>", 2) == 0) {
        direction = 1;
        corpus = get_corpus(atoi(words[1]));
        dump_ngrams(fd, corpus, direction, words, i );

    } else if (strncmp(words[0], "<:", 2) == 0) {
        direction = -1;
        corpus = get_corpus(atoi(words[1]));
        dump_ngrams(fd, corpus, direction, words, i );

    } else if (strncmp(words[0], "GET", 3) == 0) {
    	LOG("Playing http server");
    	play(fd);
    	return;
    } else {
    	ERROR(fd);
    }

}

