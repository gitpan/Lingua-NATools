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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifndef __MATRIX_H__
#define __MATRIX_H__


/**
 * @file
 * @brief Header file for the matrix data structure module
 */



/**
 * @brief increment value when enlarging a row. A bigger number makes
 * it slurp much more memory.  A smaller makes it slower 
 */
#define MEMBLOCK 8 

/**
 * @brief maximum value in one cell  (was 130)
 */
#define MAXVAL 2000000

/**
 * @brief number of significant decimals: 100 means 2  (was 500)
 */
#define MAXDEC 1000  


#include "standard.h"

/**
 * @brief Macro to return the number of rows from a Matrix
 */
#define     GetNRow(x)        (x->Nrows)

/**
 * @brief Macro to return the number of columns from a Matrix
 */
#define     GetNColumn(x)     (x->Ncolumns)

/**
 * @brief Matrix selector. The matrix structure stores two matrices at
 * a time.
 */
typedef enum {
    MATRIX_2 = 0,
    MATRIX_1 = 1
} MatrixVal;

/**
 * @brief Data structure for cell in the spare matrix
 */
typedef struct cCell {
    /** number of the column */
    guint32   column;
    /** 
     * @brief value1
     *
     * @todo explain why value1 and value2
     */
    guint32   value1;
    /** 
     * @brief value2
     *
     * @todo explain why value1 and value2
     */
    guint32   value2;
} Cell;

/**
 * @brief Data structure for each sparse matrix row
 */
typedef struct cRow {
    /** number of cells */
    guint32   length;
    /** array of cells */
    Cell     *cells;
} Row;

/**
 * @brief Main sparse matrix structure
 */
typedef struct cMatrix {
    /** number of rows in the matrix  */
    guint32   Nrows;
    /** number of columns int the matrix  */
    guint32   Ncolumns;
    /** array with pointers for each row information  */
    Row      *rows;
} Matrix;

Matrix*            AllocMatrix           (guint32       Nrow,
					  guint32       Ncolumn);

void               FreeMatrix            (Matrix       *matrix);

/*
int                PutValue              (Matrix       *matrix,
					  MatrixVal     Ma,
					  float         f,
					  guint32       r,
					  guint32       c);
*/

int                IncValue              (Matrix       *matrix,
					  MatrixVal     Ma,
					  float         incf,
					  guint32       r,
					  guint32       c);

float              GetValue              (Matrix       *matrix,
					  MatrixVal     Ma, 
					  guint32       r,
					  guint32       c);

Matrix*            LoadMatrix            (char         *filename);

int                SaveMatrix            (Matrix       *matrix,
					  char         *filename);

void               MatrixEntropy         (Matrix       *matrix,
					  MatrixVal     Ma, 
					  double       *h,
				 	  double       *hx,
					  double       *hy, 
					  double       *hygx,
					  double       *hxgy,
					  double       *uygx,
					  double       *uxgy,
					  double       *uxy);

float              GetRow                (Matrix       *matrix,
					  MatrixVal     Ma, 
					  guint32       r,
					  guint32      *c,
					  float        *f);

/*
float              GetRowMax             (Matrix       *matrix,
					  MatrixVal     Ma, 
					  guint32       r,
					  guint32      *c,
					  float        *f,
					  guint32       max);
*/

/*
float              GetColumnMax          (Matrix       *matrix,
					  MatrixVal     Ma, 
					  guint32       c,
					  guint32      *r,
					  float        *f,
					  guint32       max);
*/

int                GetPartialMatrix      (Matrix       *matrix,
					  MatrixVal     Ma, 
					  guint32      *r,
					  guint32      *c,
					  double       *M,
					  guint32       max);

/*
int                GetConditionalMatrix  (Matrix       *matrix,
					  MatrixVal     Ma, 
					  guint32      *r,
					  guint32      *c,
					  float        *M,
					  guint32       max);
*/

void               ClearMatrix           (Matrix       *matrix,
				          MatrixVal     Ma);

void               CopyMatrix            (Matrix       *matrix,
  			                  MatrixVal     Mdest);

float              CompareMatrices       (Matrix       *matrix);

/* Gives mean difference */
float              MatrixTotal           (Matrix       *matrix,
			     	          MatrixVal     Ma);

void               ColumnTotals          (Matrix       *matrix,
					  MatrixVal     Ma,
					  float        *cf);

guint32            BytesInUse            (Matrix       *matrix);

#endif /* __MATRIX_H__ */
