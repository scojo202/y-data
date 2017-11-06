/*
 * y-data-class.h :
 *
 * Copyright (C) 2003-2004 Jody Goldberg (jody@gnome.org)
 * Copyright (C) 2016 Scott O. Johnson (scojo202@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef Y_DATA_CLASS_H
#define Y_DATA_CLASS_H

#include <glib-object.h>

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(YData,y_data,Y,DATA,GInitiallyUnowned)

#define Y_TYPE_DATA	(y_data_get_type ())

/**
 * YMatrixSize:
 * @rows: rows number, includes missing values.
 * @columns: columns number, includes missing values.
 *
 * Holds the size of a matrix.
 **/

typedef struct {
	unsigned int rows;
	unsigned int columns;
} YMatrixSize;

/**
 * YThreeDArraySize:
 * @layers: number of layers, includes missing values.
 * @rows: rows number, includes missing values.
 * @columns: columns number, includes missing values.
 *
 * Holds the size of a matrix.
 **/

typedef struct {
	unsigned int layers;
	unsigned int rows;
	unsigned int columns;
} YThreeDArraySize;

/**
 * YDataClass:
 * @base: base class.
 * @n_dimensions: number of dimensions of the array (-1 if struct)
 * @dup: duplicates the #YData.
 * @serialize: serializes to text.
 * @get_sizes: gets the sizes.
 * @get_bounds: gets the bounds.
 * @emit_changed: changed signal default handler
 *
 * Class for YData.
 **/
struct _YDataClass {
  GObjectClass base;

  char n_dimensions;

  YData * (*dup)	    		(YData *src);

  char * (*serialize)	    	(YData *dat, gpointer user);

  void (*get_sizes)		(YData *data, unsigned int *sizes);
  void (*get_bounds)		(YData *data, double *minimum, double *maximum);

  /* signals */
  void (*emit_changed)	(YData *dat);
};

G_DECLARE_DERIVABLE_TYPE(YScalar,y_scalar,Y,SCALAR,YData)

#define Y_TYPE_SCALAR	(y_scalar_get_type ())

/**
 * YScalarClass:
 * @base:  base class.
 * @get_value: gets the value.
 *
 * Class for YScalar.
 **/

struct _YScalarClass {
  YDataClass base;
  double       (*get_value)  (YScalar *scalar);
};

G_DECLARE_FINAL_TYPE(YScalarVal,y_scalar_val,Y,SCALAR_VAL,YScalar)

#define Y_TYPE_SCALAR_VAL	(y_scalar_val_get_type ())

YData *y_scalar_val_new      (double val);
double *y_scalar_val_get_val (YScalarVal *s);

G_DECLARE_DERIVABLE_TYPE(YVector,y_vector,Y,VECTOR,YData)

#define Y_TYPE_VECTOR	(y_vector_get_type ())

/**
 * YVectorClass:
 * @base: base class.
 * @load_len: loads the vector length and returns it.
 * @load_values: loads the values and returns them.
 * @get_value: gets a value.
 *
 * Class for YVector.
 **/

struct _YVectorClass {
	YDataClass base;

	unsigned int	 (*load_len)    (YVector *vec);
	double	 *(*load_values) (YVector *vec);
	double	 (*get_value)   (YVector *vec, unsigned i);
};

G_DECLARE_DERIVABLE_TYPE(YMatrix,y_matrix,Y,MATRIX,YData)

#define Y_TYPE_MATRIX (y_matrix_get_type())

/**
 * YMatrixClass:
 * @base: base class.
 * @load_size: loads the matrix length.
 * @load_values: loads the values in the cache.
 * @get_value: gets a value.
 *
 * Class for YMatrix.
 **/

struct _YMatrixClass {
	YDataClass base;

	YMatrixSize	 (*load_size)    (YMatrix *vec);
	double	 *(*load_values) (YMatrix *vec);
	double	 (*get_value)   (YMatrix *mat, unsigned i, unsigned j);
};

G_DECLARE_DERIVABLE_TYPE(YThreeDArray,y_three_d_array,Y,THREE_D_ARRAY,YData)

#define Y_TYPE_THREE_D_ARRAY (y_three_d_array_get_type())

/**
 * YThreeDArrayClass:
 * @base: base class.
 * @load_size: loads the matrix length.
 * @load_values: loads the values in the cache.
 * @get_value: gets a value.
 *
 * Class for YThreeDArray.
 **/

struct _YThreeDArrayClass {
	YDataClass base;

	YThreeDArraySize	 (*load_size)    (YThreeDArray *vec);
	double	 *(*load_values) (YThreeDArray *vec);
	double	 (*get_value)   (YThreeDArray *mat, unsigned i, unsigned j, unsigned k);
};

YData *	y_data_dup (YData *src);
YData * y_data_dup_to_simple (YData *src);

char * y_data_serialize (YData *dat, gpointer user);
void y_data_emit_changed (YData *dat);

void		y_data_get_bounds		(YData *data, double *minimum, double *maximum);
gboolean	y_data_has_value	    (YData *data);

char 	y_data_get_n_dimensions 	(YData *data);
unsigned int	y_data_get_n_values		(YData *data);

/*************************************************************************/

double      y_scalar_get_value  (YScalar *scalar);
char *y_scalar_get_str (YScalar *scalar, const gchar *format);

/*************************************************************************/

unsigned int	 y_vector_get_len    (YVector *vec);
const double	*y_vector_get_values (YVector *vec);
double	 y_vector_get_value  (YVector *vec, unsigned i);
char *	 y_vector_get_str  (YVector *vec, unsigned int i, const gchar *format);
gboolean	y_vector_is_varying_uniformly	(YVector *data);
void	 y_vector_get_minmax (YVector *vec, double *min, double *max);
gboolean y_vector_vary_uniformly (YVector *vec);

/*************************************************************************/

YMatrixSize	 y_matrix_get_size    (YMatrix *mat);
unsigned int 	 y_matrix_get_rows   (YMatrix *mat);
unsigned int 	 y_matrix_get_columns (YMatrix *mat);
const double	*y_matrix_get_values (YMatrix *mat);
double	 y_matrix_get_value  (YMatrix *mat, unsigned i, unsigned j);
char *y_matrix_get_str (YMatrix *mat, unsigned i, unsigned j, const gchar *format);
void	 y_matrix_get_minmax (YMatrix *mat, double *min, double *max);

/*************************************************************************/

YThreeDArraySize	 y_three_d_array_get_size    (YThreeDArray *mat);
unsigned int 	 y_three_d_array_get_rows   (YThreeDArray *mat);
unsigned int 	 y_three_d_array_get_columns (YThreeDArray *mat);
unsigned int 	 y_three_d_array_get_layers (YThreeDArray *mat);
const double	*y_three_d_array_get_values (YThreeDArray *mat);
double	 y_three_d_array_get_value  (YThreeDArray *mat, unsigned i, unsigned j, unsigned k);
char *y_three_d_array_get_str (YThreeDArray *mat, unsigned i, unsigned j, unsigned k, const gchar *format);
void	 y_three_d_array_get_minmax (YThreeDArray *mat, double *min, double *max);

/*************************************************************************/

G_DECLARE_FINAL_TYPE(YStruct,y_struct,Y,STRUCT,YData)

#define Y_TYPE_STRUCT (y_struct_get_type())

YData *y_struct_get_data(YStruct     *s,
                         const gchar *name);
void y_struct_set_data(YStruct     *s,
                         const gchar *name, YData *d);
void y_struct_foreach(YStruct   *s,
                      GHFunc  f,
                      gpointer   user_data);

G_END_DECLS

#endif /* Y_DATA_H */
