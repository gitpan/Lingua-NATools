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

#ifndef __TEMPDICT_H__
#define __TEMPDICT_H__


/**
 * @file
 * @brief Header file for temporary dictionary created during 
 *             co-occurrence matrix interpretation
 */

/**
 * @brief ??
 *
 * @todo Understand this and document
 */
struct cItem {
    /** next column  */
    struct cItem *nextC;
    /** row number  */
    guint32       row;
    /** column number  */
    guint32       column;
    /** cell value  */
    guint32       value;
};

/**
 * @brief ??
 *
 * @todo Understand this and document
 */
struct cMat2 {
    /** Number of rows */
    guint32        Nrows;
    /** Number of columns */
    guint32        Ncolumns;    
    /** list of pointers to first item per row */
    struct cItem **firstR;
    /** list of pointers to first item per column */
    struct cItem **firstC;      
    /** sparse data structure */
    struct cItem  *items;       
    /** boolean 0 = matrix is dirty, 1 = matrix = clean */ 
    guint32        clean;       
    /** ??  */
    long           p;
    /** pointer to empty space & memory available */
    long           memory;      
};

int          tempdict_allocmatrix2         (struct cMat2    *Matrix,
					    guint32          Nrow,
					    guint32          Ncolumn);

void         tempdict_freematrix2          (struct cMat2    *Matrix);

int          tempdict_dirtyputvalue2       (struct cMat2    *Matrix,
				   float            f,
				   guint32          r,
				   guint32          c);

float        tempdict_getrowmax2           (struct cMat2    *Matrix,
					    guint32          r,
					    guint32         *c,
					    float           *f,
					    guint32          max);

float        tempdict_getcolumnmax2        (struct cMat2    *Matrix,
					    guint32          c,
					    guint32         *r,
					    float           *f,
					    guint32          max);

int          tempdict_loadmatrix2          (struct cMat2    *Matrix,
					    const char      *filename);

int          tempdict_savematrix2          (struct cMat2    *Matrix,
					    const char      *filename);

#endif
