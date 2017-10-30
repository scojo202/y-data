/*
 * y-data-simple.c :
 *
 * Copyright (C) 2003-2005 Jody Goldberg (jody@gnome.org)
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
 
#include "y-data-simple.h"
#include <math.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>

/**
 * SECTION: y-data-simple
 * @short_description: Data objects based on simple arrays.
 *
 * Data classes #YScalarVal, #YVectorVal, and #YMatrixVal.
 *
 * In these objects, an array (or, in the case of a #YScalarVal, a single double
 * precision value) is maintained that is also the data cache. Therefore, the
 * array should not be freed.
 */

static char *
render_val (double val)
{
		char buf[G_ASCII_DTOSTR_BUF_SIZE];
		g_ascii_dtostr (buf, G_ASCII_DTOSTR_BUF_SIZE, val);
		return g_strdup (buf);
}

struct _YScalarVal {
	YScalar      base;
	double		 val;
};

G_DEFINE_TYPE (YScalarVal, y_scalar_val, Y_TYPE_SCALAR);

static void
y_scalar_val_finalize (GObject *obj)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(y_scalar_val_parent_class);

	(*obj_class->finalize) (obj);
}

static YData *
y_scalar_val_dup (YData *src)
{
	YScalarVal *dst = g_object_new (G_OBJECT_TYPE (src), NULL);
	YScalarVal const *src_val = (YScalarVal const *)src;
	dst->val = src_val->val;
	return Y_DATA (dst);
}

static char *
y_scalar_val_serialize (YData *dat, gpointer user)
{
	YScalarVal *sval = (YScalarVal *)dat;
	return render_val (sval->val);
}

static gboolean
y_scalar_val_unserialize (YData *dat, char const *str, gpointer user)
{
	YScalarVal *sval = (YScalarVal *)dat;
	double tmp;
	char *end;
	errno = 0; /* strto(ld) sets errno, but does not clear it.  */
	tmp = g_ascii_strtod (str, &end);

	if (end == str || *end != '\0' || errno == ERANGE)
		return FALSE;

	sval->val = tmp;
	return TRUE;
}

static double
y_scalar_val_get_value (YScalar *dat)
{
	YScalarVal const *sval = (YScalarVal const *)dat;
	return sval->val;
}

static void
y_scalar_val_class_init (YScalarValClass *scalarval_klass)
{
	YDataClass *ydata_klass = (YDataClass *) scalarval_klass;
	YScalarClass *scalar_klass = (YScalarClass *) scalarval_klass;
	GObjectClass *gobject_klass = (GObjectClass *) scalarval_klass;

	gobject_klass->finalize   = y_scalar_val_finalize;
	ydata_klass->dup	  = y_scalar_val_dup;
	ydata_klass->serialize	  = y_scalar_val_serialize;
	ydata_klass->unserialize = y_scalar_val_unserialize;
	scalar_klass->get_value	  = y_scalar_val_get_value;
}

static void
y_scalar_val_init(YScalarVal *val) {}

/**
 * y_scalar_val_new:
 * @val: initial value
 *
 * Creates a new #YScalarVal object.
 *
 * Returns: (transfer full): The new object.
 **/
YData *
y_scalar_val_new (double val)
{
	YScalarVal *res = g_object_new (Y_TYPE_SCALAR_VAL, NULL);
	res->val = val;

	return Y_DATA (res);
}

/**
 * y_scalar_val_get_val:
 * @s: a #YScalarVal
 *
 * Gets a pointer to the value of a #YScalarVal.
 *
 * Returns: (transfer none): A pointer to the scalar value.
 **/
double * y_scalar_val_get_val (YScalarVal *s)
{
  return &s->val;
}

/*****************************************************************************/

struct _YVectorVal {
	YVector	 base;
	unsigned	 n;
	double *val;
	GDestroyNotify notify;
};

G_DEFINE_TYPE (YVectorVal, y_vector_val, Y_TYPE_VECTOR);

static void
y_vector_val_finalize (GObject *obj)
{
	YVectorVal *vec = (YVectorVal *)obj;
	if (vec->notify && vec->val)
		(*vec->notify) (vec->val);

	GObjectClass *obj_class = G_OBJECT_CLASS(y_vector_val_parent_class);

	(*obj_class->finalize) (obj);
}

static YData *
y_vector_val_dup (YData *src)
{
	YVectorVal *dst = g_object_new (G_OBJECT_TYPE (src), NULL);
	YVectorVal const *src_val = (YVectorVal const *)src;
	if (src_val->notify) {
		dst->val = g_new0 (double, src_val->n);
		memcpy (dst->val, src_val->val, src_val->n * sizeof (double));
		dst->notify = g_free;
	} else
		dst->val = src_val->val;
	dst->n = src_val->n;
	return Y_DATA (dst);
}

static unsigned int
y_vector_val_load_len (YVector *vec)
{
	return ((YVectorVal *)vec)->n;
}

static double *
y_vector_val_load_values (YVector *vec)
{
	YVectorVal const *val = (YVectorVal const *)vec;

	return val->val;
}

static double
y_vector_val_get_value (YVector *vec, unsigned i)
{
	YVectorVal const *val = (YVectorVal const *)vec;
	g_return_val_if_fail (val != NULL && val->val != NULL && i < val->n, NAN);
	return val->val[i];
}

static char *
y_vector_val_serialize (YData *dat, gpointer user)
{
	YVectorVal const *vec = (YVectorVal const *) dat;
	GString *str;
	char sep;
	unsigned i;

	sep = '\t';
	str = g_string_new (NULL);

	for (i = 0; i < vec->n; i++) {
		char *s = render_val (vec->val[i]);
		if (i) g_string_append_c (str, sep);
		g_string_append (str, s);
		g_free (s);
	}
	return g_string_free (str, FALSE);
}

static gboolean
y_vector_val_unserialize (YData *dat, char const *str, gpointer user)
{
	YVectorVal *vec = Y_VECTOR_VAL (dat);
	char sep, *end = (char*) str;
	double val;
	GArray *values;

	g_return_val_if_fail (str != NULL, TRUE);

	if (vec->notify && vec->val)
		(*vec->notify) (vec->val);

	values = g_array_sized_new (FALSE, FALSE, sizeof(double), 16);
	sep = 0;
	vec->val = NULL;
	vec->n = 0;
	vec->notify = (GDestroyNotify) g_free;
	while (1) {
		val = g_ascii_strtod (end, &end);
		g_array_append_val (values, val);
		if (*end) {
			if (!sep) {
				/* allow the use of all possible seps */
				if ((sep = ',') != *end)
					if ((sep = '\t') != *end)
						sep = '\n';
			}
			if (*end != sep) {
				g_array_free (values, TRUE);
				return FALSE;
			}
			end++;
		} else
			break;
	}
	if (values->len == 0) {
		g_array_free (values, TRUE);
		return TRUE;
	}
	vec->n = values->len;
	vec->val = (double*) values->data;
	g_array_free (values, FALSE);
	y_data_emit_changed (Y_DATA (vec));
	return TRUE;
}

static void
y_vector_val_class_init (YVectorValClass *val_klass)
{
	YDataClass *ydata_klass = (YDataClass *) val_klass;
	YVectorClass *vector_klass = (YVectorClass *) val_klass;
	GObjectClass *gobject_klass = (GObjectClass *) val_klass;

	gobject_klass->finalize = y_vector_val_finalize;
	ydata_klass->dup	= y_vector_val_dup;
	ydata_klass->serialize	= y_vector_val_serialize;
	ydata_klass->unserialize	= y_vector_val_unserialize;
	vector_klass->load_len    = y_vector_val_load_len;
	vector_klass->load_values = y_vector_val_load_values;
	vector_klass->get_value   = y_vector_val_get_value;
}

static void
y_vector_val_init(YVectorVal *val) {}

/**
 * y_vector_val_new: (skip)
 * @val: (array length=n): array of doubles
 * @n: length of array
 * @notify: (nullable): the function to be called to free the array when the #YData is unreferenced, or %NULL
 *
 * Returns: a #YData
 **/

YData *
y_vector_val_new (double *val, unsigned n, GDestroyNotify notify)
{
	YVectorVal *res = g_object_new (Y_TYPE_VECTOR_VAL, NULL);
	res->val = val;
	res->n = n;
	res->notify = notify;
	return Y_DATA (res);
}

/**
 * y_vector_val_new_alloc:
 * @n: length of array
 *
 * Returns: a #YData
 **/
YData *
y_vector_val_new_alloc (unsigned n)
{
        YVectorVal *res = g_object_new (Y_TYPE_VECTOR_VAL, NULL);
	res->val = g_malloc0(sizeof(double)*n);
	res->n = n;
	res->notify = g_free;
	return Y_DATA (res);
}

/**
 * y_vector_val_new_copy:
 * @val: (array length=n): array of doubles
 * @n: length of array
 *
 * Returns: a #YData
 **/

YData *
y_vector_val_new_copy (double *val, unsigned n)
{
	double *val2 = g_memdup (val, sizeof(double)*n);
	return y_vector_val_new(val2,n,g_free);
}

/**
 * y_vector_val_replace_array :
 * @s: #YVectorVal
 * @array: (array length=n): array of doubles
 * @n: length of array
 * @notify: (nullable): the function to be called to free the array when the #YData is unreferenced, or %NULL
 *
 * Get the array of values of @vec. 
 *
 * Returns: an array. Should not be freed.
 **/
void y_vector_val_replace_array(YVectorVal *s, double *array, unsigned n, GDestroyNotify notify)
{
  if(s->val && s->notify)
    (*s->notify)(s->val);
  s->val = array;
  s->n = n;
  s->notify = notify;
  y_data_emit_changed(Y_DATA(s));
}

/**
 * y_vector_val_get_array :
 * @s: #YVectorVal
 *
 * Get the array of values of @vec. 
 *
 * Returns: an array. Should not be freed.
 **/
double *y_vector_val_get_array (YVectorVal *s)
{
  return s->val;
}

/*****************************************************************************/

struct _YMatrixVal {
	YMatrix	 base;
	YMatrixSize size;
	double *val;
	GDestroyNotify notify;
};

G_DEFINE_TYPE (YMatrixVal, y_matrix_val, Y_TYPE_MATRIX);

static void
y_matrix_val_finalize (GObject *obj)
{
	YMatrixVal *mat = (YMatrixVal *)obj;
	if (mat->notify && mat->val)
		(*mat->notify) (mat->val);

	G_OBJECT_CLASS(y_matrix_val_parent_class)->finalize (obj);
}

static YData *
y_matrix_val_dup (YData *src)
{
	YMatrixVal *dst = g_object_new (G_OBJECT_TYPE (src), NULL);
	YMatrixVal const *src_val = (YMatrixVal const *)src;
	if (src_val->notify) {
		dst->val = g_new (double, src_val->size.rows * src_val->size.columns);
		memcpy (dst->val, src_val->val, src_val->size.rows * src_val->size.columns * sizeof (double));
		dst->notify = g_free;
	} else
		dst->val = src_val->val;
	dst->size = src_val->size;
	return Y_DATA (dst);
}

static YMatrixSize
y_matrix_val_load_size (YMatrix *mat)
{
	return ((YMatrixVal *)mat)->size;
}

static double *
y_matrix_val_load_values (YMatrix *mat)
{
	YMatrixVal const *val = (YMatrixVal const *)mat;
  return val->val;
}

static double
y_matrix_val_get_value (YMatrix *mat, unsigned i, unsigned j)
{
	YMatrixVal const *val = (YMatrixVal const *)mat;

	return val->val[i * val->size.columns + j];
}

static char *
y_matrix_val_serialize (YData *dat, gpointer user)
{
	YMatrixVal const *mat = (YMatrixVal const *) dat;
	GString *str;
	size_t c, r;
	char col_sep = '\t';
	char row_sep = '\n';

	str = g_string_new (NULL);
	for (r = 0; r < mat->size.rows; r++) {
		if (r) g_string_append_c (str, row_sep);
		for (c = 0; c < mat->size.columns; c++) {
			double val = mat->val[r * mat->size.columns + c];
			char *s = render_val (val);
			if (c) g_string_append_c (str, col_sep);
			g_string_append (str, s);
			g_free (s);
		}
	}

	return g_string_free (str, FALSE);
}

static gboolean
y_matrix_val_unserialize (YData *dat, char const *str, gpointer user)
{
	YMatrixVal *mat = Y_MATRIX_VAL (dat);
	char row_sep, col_sep, *end = (char*) str;
	int i, j, columns;
	double val;
	GArray *values;

	g_return_val_if_fail (str != NULL, TRUE);

	values = g_array_sized_new (FALSE, FALSE, sizeof(double), 16);
	col_sep = '\t';
	row_sep = '\n';
	i = j = columns = 0;
	if (mat->notify && mat->val)
		(*mat->notify) (mat->val);
	mat->val = NULL;
	mat->size.rows = 0;
	mat->size.columns = 0;
	mat->notify = g_free;
	while (1) {
		val = g_ascii_strtod (end, &end);
		g_array_append_val (values, val);
		if (*end) {
			if (*end == col_sep)
				j++;
			else if (*end == row_sep) {
				if (columns > 0) {
					if (j == columns - 1) {
						i++;
						j = 0;
					} else {
						g_array_free (values, TRUE);
						return FALSE;
					}
				} else {
					columns = j + 1;
					i++;
					j = 0;
				}
			} else {
				g_array_free (values, TRUE);
				return FALSE;
			}
			end++;
		} else
			break;
	}
	if (j != columns - 1) {
		g_array_free (values, TRUE);
		return FALSE;
	}
	if (columns == 0) {
		g_array_free (values, TRUE);
		return TRUE;
	}
	mat->size.columns = columns;
	mat->size.rows = i + 1;
	mat->val = (double*) values->data;
	g_array_free (values, FALSE);
	y_data_emit_changed (Y_DATA (mat));
	return TRUE;
}

static void
y_matrix_val_class_init (YMatrixValClass *val_klass)
{
	GObjectClass *gobject_klass = (GObjectClass *) val_klass;
	YDataClass *ydata_klass = (YDataClass *) gobject_klass;
	YMatrixClass *matrix_klass = (YMatrixClass *) gobject_klass;

	gobject_klass->finalize = y_matrix_val_finalize;
	ydata_klass->dup	= y_matrix_val_dup;
	ydata_klass->serialize	= y_matrix_val_serialize;
	ydata_klass->unserialize = y_matrix_val_unserialize;
	matrix_klass->load_size   = y_matrix_val_load_size;
	matrix_klass->load_values = y_matrix_val_load_values;
	matrix_klass->get_value   = y_matrix_val_get_value;
}

static void
y_matrix_val_init(YMatrixVal *val) {}

/**
 * y_matrix_val_new: (skip)
 * @val: array of doubles
 * @rows: number of rows
 * @columns: number of columns
 * @notify: (nullable): the function to be called to free the array when the #YData is unreferenced, or %NULL
 *
 * Returns: a #YData
 **/
YData *
y_matrix_val_new (double *val, unsigned rows, unsigned columns, GDestroyNotify   notify)
{
	YMatrixVal *res = g_object_new (Y_TYPE_MATRIX_VAL, NULL);
	res->val = val;
	res->size.rows = rows;
	res->size.columns = columns;
	res->notify = notify;
	return Y_DATA (res);
}

/**
 * y_matrix_val_new_copy:
 * @val: array of doubles with at least @rows*@columns elements
 * @rows: number of rows
 * @columns: number of columns
 *
 * Returns: a #YData
 **/
YData *y_matrix_val_new_copy (double   *val,
                                     unsigned  rows, unsigned columns)
{
  return y_matrix_val_new(g_memdup(val,sizeof(double)*rows*columns),rows,columns,g_free);
}

/**
 * y_matrix_val_new_alloc:
 * @rows: number of rows
 * @columns: number of columns
 *
 * Allocate a new array with @rows rows and @columns columns and use it in a new #YMatrixVal.
 *
 * Returns: a #YData
 **/
YData *y_matrix_val_new_alloc (unsigned rows, unsigned columns)
{
  YMatrixVal *res = g_object_new (Y_TYPE_MATRIX_VAL, NULL);
	res->val = g_new0(double, rows*columns);
	res->size.rows = rows;
	res->size.columns = columns;
	res->notify = g_free;
	return Y_DATA (res);
}

/**
 * y_matrix_val_get_array :
 * @s: #YVectorVal
 *
 * Get the array of values of @s. 
 *
 * Returns: an array. Should not be freed.
 **/

double *y_matrix_val_get_array (YMatrixVal *s)
{
  return s->val;
}

/********************************************/

/**
 * y_data_dup_to_simple:
 * @src: #YData
 *
 * Duplicates a #YData object.
 *
 * Returns: (transfer full): A deep copy of @src.
 **/
YData *y_data_dup_to_simple(YData *src)
{
  return NULL;
}

/*****************************************************************************/

struct _YThreeDArrayVal {
	YThreeDArray	 base;
	YThreeDArraySize size;
	double *val;
	GDestroyNotify notify;
};

G_DEFINE_TYPE (YThreeDArrayVal, y_three_d_array_val, Y_TYPE_THREE_D_ARRAY);

static void
y_three_d_array_val_finalize (GObject *obj)
{
	YThreeDArrayVal *mat = (YThreeDArrayVal *)obj;
	if (mat->notify && mat->val)
		(*mat->notify) (mat->val);

	G_OBJECT_CLASS(y_three_d_array_val_parent_class)->finalize (obj);
}

static YData *
y_three_d_array_val_dup (YData *src)
{
	YThreeDArrayVal *dst = g_object_new (G_OBJECT_TYPE (src), NULL);
	YThreeDArrayVal const *src_val = (YThreeDArrayVal const *)src;
	if (src_val->notify) {
		dst->val = g_new (double, src_val->size.rows * src_val->size.columns * src_val->size.layers);
		memcpy (dst->val, src_val->val, src_val->size.rows * src_val->size.columns * src_val->size.layers * sizeof (double));
		dst->notify = g_free;
	} else
		dst->val = src_val->val;
	dst->size = src_val->size;
	return Y_DATA (dst);
}

static YThreeDArraySize
y_three_d_array_val_load_size (YThreeDArray *mat)
{
	return ((YThreeDArrayVal *)mat)->size;
}

static double *
y_three_d_array_val_load_values (YThreeDArray *mat)
{
	YThreeDArrayVal const *val = (YThreeDArrayVal const *)mat;
  return val->val;
}

static double
y_three_d_array_val_get_value (YThreeDArray *mat, unsigned i, unsigned j, unsigned k)
{
	YThreeDArrayVal const *val = (YThreeDArrayVal const *)mat;

	return val->val[i * val->size.columns * val->size.rows + j * val->size.columns + k];
}

#if 0

static char *
y_three_d_array_val_get_str (YThreeDArray *mat, unsigned i, unsigned j, unsigned k)
{
	YThreeDArrayVal const *val = (YThreeDArrayVal const *)mat;

	return render_val (val->val[i * val->size.columns + j]);
}

static char *
y_three_d_array_val_serialize (YData const *dat, gpointer user)
{
	YThreeDArrayVal *mat = Y_MATRIX_VAL (dat);
	GString *str;
	size_t c, r;
	char col_sep = '\t';
	char row_sep = '\n';

	str = g_string_new (NULL);
	for (r = 0; r < mat->size.rows; r++) {
		if (r) g_string_append_c (str, row_sep);
		for (c = 0; c < mat->size.columns; c++) {
			double val = mat->val[r * mat->size.columns + c];
			char *s = render_val (val);
			if (c) g_string_append_c (str, col_sep);
			g_string_append (str, s);
			g_free (s);
		}
	}

	return g_string_free (str, FALSE);
}

static gboolean
y_three_d_array_val_unserialize (YData *dat, char const *str, gpointer user)
{
	YThreeDArrayVal *mat = Y_MATRIX_VAL (dat);
	char row_sep, col_sep, *end = (char*) str;
	int i, j, columns;
	double val;
	GArray *values;

	g_return_val_if_fail (str != NULL, TRUE);

	values = g_array_sized_new (FALSE, FALSE, sizeof(double), 16);
	col_sep = '\t';
	row_sep = '\n';
	i = j = columns = 0;
	if (mat->notify && mat->val)
		(*mat->notify) (mat->val);
	mat->val = NULL;
	mat->size.rows = 0;
	mat->size.columns = 0;
	mat->notify = g_free;
	while (1) {
		val = g_ascii_strtod (end, &end);
		g_array_append_val (values, val);
		if (*end) {
			if (*end == col_sep)
				j++;
			else if (*end == row_sep) {
				if (columns > 0) {
					if (j == columns - 1) {
						i++;
						j = 0;
					} else {
						g_array_free (values, TRUE);
						return FALSE;
					}
				} else {
					columns = j + 1;
					i++;
					j = 0;
				}
			} else {
				g_array_free (values, TRUE);
				return FALSE;
			}
			end++;
		} else
			break;
	}
	if (j != columns - 1) {
		g_array_free (values, TRUE);
		return FALSE;
	}
	if (columns == 0) {
		g_array_free (values, TRUE);
		return TRUE;
	}
	mat->size.columns = columns;
	mat->size.rows = i + 1;
	mat->val = (double*) values->data;
	g_array_free (values, FALSE);
	y_data_emit_changed (Y_DATA (mat));
	return TRUE;
}
#endif

static void
y_three_d_array_val_class_init (YThreeDArrayValClass *val_klass)
{
	GObjectClass *gobject_klass = (GObjectClass *) val_klass;
	YDataClass *ydata_klass = (YDataClass *) gobject_klass;
	YThreeDArrayClass *matrix_klass = (YThreeDArrayClass *) gobject_klass;

	gobject_klass->finalize = y_three_d_array_val_finalize;
	ydata_klass->dup	= y_three_d_array_val_dup;
	//ydata_klass->serialize	= y_three_d_array_val_serialize;
	//ydata_klass->unserialize = y_three_d_array_val_unserialize;
	matrix_klass->load_size   = y_three_d_array_val_load_size;
	matrix_klass->load_values = y_three_d_array_val_load_values;
	matrix_klass->get_value   = y_three_d_array_val_get_value;
}

static void
y_three_d_array_val_init(YThreeDArrayVal *val) {}

/**
 * y_three_d_array_val_new: (skip)
 * @val: array of doubles
 * @rows: number of rows
 * @columns: number of columns
 * @notify: (nullable): the function to be called to free the array when the #YData is unreferenced, or %NULL
 *
 * Returns: a #YData
 **/
YData *
y_three_d_array_val_new (double *val, unsigned rows, unsigned columns, unsigned layers, GDestroyNotify   notify)
{
	YThreeDArrayVal *res = g_object_new (Y_TYPE_THREE_D_ARRAY_VAL, NULL);
	res->val = val;
	res->size.rows = rows;
	res->size.columns = columns;
	res->size.layers = layers;
	res->notify = notify;
	return Y_DATA (res);
}

/**
 * y_three_d_array_val_new_copy:
 * @val: array of doubles with at least @rows*@columns elements
 * @rows: number of rows
 * @columns: number of columns
 *
 * Returns: a #YData
 **/
YData *y_three_d_array_val_new_copy (double   *val,
                                     unsigned  rows, unsigned columns, unsigned layers)
{
  return y_three_d_array_val_new(g_memdup(val,sizeof(double)*rows*columns*layers),rows,columns,layers,g_free);
}

/**
 * y_three_d_array_val_new_alloc:
 * @rows: number of rows
 * @columns: number of columns
 *
 * Allocate a new array with @rows rows and @columns columns and use it in a new #YThreeDArrayVal.
 *
 * Returns: a #YData
 **/
YData *y_three_d_array_val_new_alloc (unsigned rows, unsigned columns, unsigned layers)
{
  YThreeDArrayVal *res = g_object_new (Y_TYPE_THREE_D_ARRAY_VAL, NULL);
	res->val = g_new0(double, rows*columns);
	res->size.rows = rows;
	res->size.columns = columns;
	res->size.layers = layers;
	res->notify = g_free;
	return Y_DATA (res);
}

/**
 * y_three_d_array_val_get_array :
 * @s: #YVectorVal
 *
 * Get the array of values of @s. 
 *
 * Returns: an array. Should not be freed.
 **/

double *y_three_d_array_val_get_array (YThreeDArrayVal *s)
{
  return s->val;
}

