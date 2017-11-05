/*
 * y-data.c :
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

#include "y-data-class.h"
#include "y-operation.h"
#include <math.h>
#include <string.h>
#include <errno.h>

/**
 * SECTION: y-data
 * @short_description: Base class for data objects.
 *
 * Abstract base class for data classes #YScalar,
 * #YVector, and #YMatrix, representing numbers and arrays of numbers.
 *
 * Data objects maintain a cache of the values for fast access. When the
 * underlying data changes, the "changed" signal is emitted, and the default
 * signal handler invalidates the cache. Subsequent calls to "get_values" will
 * refill the cache.
 */


/**
 * YDataFlags:
 *
 * Various flags used internally.
 *
 * @Y_DATA_CACHE_IS_VALID: data in cache are valid.
 * @Y_DATA_IS_EDITABLE: data can be edited.
 * @Y_DATA_SIZE_CACHED: cached size is valid.
 * @Y_DATA_HAS_VALUE: data has at least one finite value.
 * @Y_DATA_MINMAX_CACHED: bounds cache is valid.
 **/

typedef enum {
	Y_DATA_CACHE_IS_VALID =	1 << 0,
	Y_DATA_IS_EDITABLE =		1 << 1,
	Y_DATA_SIZE_CACHED =		1 << 2,
	Y_DATA_HAS_VALUE =      1 << 3,
	Y_DATA_MINMAX_CACHED =  1 << 4
} YDataFlags;

typedef struct {
  gint32		flags;
} YDataPrivate;

enum {
	CHANGED,
	LAST_SIGNAL
};

static gulong y_data_signals [LAST_SIGNAL] = { 0, };

static char *
render_val (double val)
{
		char buf[G_ASCII_DTOSTR_BUF_SIZE];
		g_ascii_dtostr (buf, G_ASCII_DTOSTR_BUF_SIZE, val);
		return g_strdup (buf);
}

/**
 * YData:
 *
 * Object representing data.
 **/

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (YData, y_data, G_TYPE_INITIALLY_UNOWNED);

static void
y_data_init (YData *data)
{
  YDataPrivate *priv = y_data_get_instance_private(data);
  priv->flags = 0;
}

static void
y_data_class_init (YDataClass *klass)
{
/**
 * YData::changed:
 * @YData: the data object that changed
 *
 * The ::changed signal is emitted when the data changes.
 */

	y_data_signals [CHANGED] = g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
		G_STRUCT_OFFSET (YDataClass, emit_changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);
}

/**
 * y_data_dup:
 * @src: #YData
 *
 * Duplicates a #YData object.
 *
 * Returns: (transfer full): A deep copy of @src.
 **/
YData *
y_data_dup (YData *src)
{
	if (src != NULL) {
		YDataClass *klass = Y_DATA_GET_CLASS (src);
		g_return_val_if_fail (klass != NULL, NULL);
		return (*klass->dup) (src);
	}
	return NULL;
}

/**
 * y_data_serialize :
 * @dat: #YData
 * @user: a gpointer describing the context.
 *
 * Returns: a string representation of the data that the caller is
 * 	responsible for freeing
 **/
char *
y_data_serialize (YData *dat, gpointer user)
{
	YDataClass *klass = Y_DATA_GET_CLASS (dat);
	g_return_val_if_fail (klass != NULL, NULL);
	return (*klass->serialize) (dat, user);
}

/**
 * y_data_emit_changed :
 * @dat: #YData
 *
 * Utility to emit a 'changed' signal
 **/
void
y_data_emit_changed (YData *dat)
{
	YDataClass *klass = Y_DATA_GET_CLASS (dat);

	g_return_if_fail (klass != NULL);

	g_signal_emit (G_OBJECT (dat), y_data_signals [CHANGED], 0);
}

/**
 * y_data_has_value :
 * @data: #YData
 *
 * Returns whether @data contains a finite value.
 *
 * Returns: %TRUE if @data has at least one finite value.
 **/
gboolean
y_data_has_value (YData *data)
{
	g_return_val_if_fail (Y_IS_DATA (data), FALSE);
	YDataPrivate *priv = y_data_get_instance_private(data);
	if (!(priv->flags & Y_DATA_MINMAX_CACHED))
		y_data_get_bounds (Y_DATA (data),NULL,NULL);
	return priv->flags & Y_DATA_HAS_VALUE;
}

/**
 * y_data_get_n_dimensions :
 * @data: #YData
 *
 * Get the number of dimensions in @data, i.e. 0 for a scalar, 1 for a vector,
 * and 2 for a matrix. Returns -1 for a struct.
 *
 * Returns: the number of dimensions
 **/
char
y_data_get_n_dimensions (YData *data)
{
	YDataClass *data_class;

	g_return_val_if_fail (Y_IS_DATA (data), 0);

	data_class = Y_DATA_GET_CLASS (data);

	return data_class->n_dimensions;
}

/**
 * y_data_get_n_values :
 * @data: #YData
 *
 * Get the number of values in @data.
 *
 * Returns: the number of elements
 **/
unsigned int
y_data_get_n_values (YData *data)
{
	YDataClass const *data_class;
	unsigned int n_values;
	unsigned int n_dimensions;
	unsigned int *sizes;
	unsigned int i;

	g_return_val_if_fail (Y_IS_DATA (data), 0);

	data_class = Y_DATA_GET_CLASS (data);

	n_dimensions = data_class->n_dimensions;
	if (n_dimensions < 1)
		return 1;

	sizes = g_newa (unsigned int, n_dimensions);

	g_return_val_if_fail (data_class->get_sizes != NULL, 0);

	data_class->get_sizes (data, sizes);

	n_values = 1;
	for (i = 0; i < n_dimensions; i++)
		n_values *= sizes[i];

	return n_values;
}

/**
 * y_data_get_bounds :
 * @data: #YData
 * @minimum: (out)(nullable): return location for minimum value, or @NULL
 * @maximum: (out)(nullable): return location for maximum value, or @NULL
 *
 * Get the minimum and maximum values in @data.
 **/
void
y_data_get_bounds (YData *data, double *minimum, double *maximum)
{
	YDataClass const *data_class;

	g_return_if_fail (Y_IS_DATA (data));

	data_class = Y_DATA_GET_CLASS (data);

	g_return_if_fail (data_class->get_bounds != NULL);

	data_class->get_bounds (data, minimum, maximum);
}

/*************************************************************************/

/**
 * SECTION: y-scalar
 * @short_description: Base class for scalar data objects.
 *
 * Abstract base class for data classes representing scalar values.
 */

typedef struct {
  double value;
} YScalarPrivate;

/**
 * YScalar:
 *
 * Object representing a single number.
 **/

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (YScalar, y_scalar, Y_TYPE_DATA);

static void
_data_scalar_get_bounds (YData *data, double *minimum, double *maximum)
{
  YScalar *scalar = (YScalar *) data;

  YDataPrivate *dpriv = y_data_get_instance_private(data);

  double v = y_scalar_get_value (scalar);

  if(minimum)
    *minimum = v;
  if(maximum)
    *maximum = v;

  if(isfinite(v)) {
    dpriv->flags |= Y_DATA_HAS_VALUE;
  }
}

static char *
_scalar_serialize (YData *dat, gpointer user)
{
  YScalar *scalar = (YScalar *)dat;
  YScalarPrivate *priv = y_scalar_get_instance_private(scalar);
  return render_val (priv->value);
}

static void
_data_scalar_emit_changed (YData *data)
{
  YDataPrivate *priv = y_data_get_instance_private(data);
  priv->flags &= ~(Y_DATA_CACHE_IS_VALID | Y_DATA_HAS_VALUE);
}

static void
y_scalar_class_init (YScalarClass *scalar_class)
{
  YDataClass *data_class = Y_DATA_CLASS(scalar_class);
  data_class->n_dimensions = 0;
  data_class->get_bounds = _data_scalar_get_bounds;
  data_class->serialize	= _scalar_serialize;
  data_class->emit_changed = _data_scalar_emit_changed;
}

static void
y_scalar_init (YScalar *scalar) {}

/**
 * y_scalar_get_value :
 * @scalar: #YScalar
 *
 * Get the value of @scalar.
 *
 * Returns: the value
 **/
double
y_scalar_get_value (YScalar *scalar)
{
  YScalarClass const *klass = Y_SCALAR_GET_CLASS (scalar);
  g_return_val_if_fail (klass != NULL, NAN);
  YDataPrivate *dpriv = y_data_get_instance_private(Y_DATA(scalar));
  YScalarPrivate *priv = y_scalar_get_instance_private(scalar);
  if (! (dpriv->flags & Y_DATA_CACHE_IS_VALID)) {
    priv->value = (*klass->get_value) (scalar);
  }
  return priv->value;
}

/**
 * y_scalar_get_str :
 * @scalar: #YScalar
 * @format: a format string to use
 *
 * Get a string representation of @scalar.
 *
 * Returns: the string. The caller is
 * 	responsible for freeing it.
 **/
char *
y_scalar_get_str (YScalar *scalar, const gchar *format)
{
  char buf[G_ASCII_DTOSTR_BUF_SIZE];
  double val = y_scalar_get_value (scalar);
  g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, format, val);
  return g_strdup (buf);
}

/**********************************************************/

/**
 * YScalarVal:
 * @base: base.
 *
 * Object holding a single double precision number.
 **/

struct _YScalarVal {
  YScalar base;
};

G_DEFINE_TYPE (YScalarVal, y_scalar_val, Y_TYPE_SCALAR);

static YData *
y_scalar_val_dup (YData *src)
{
  YScalarVal *dst = g_object_new (G_OBJECT_TYPE (src), NULL);
  YScalarPrivate *priv = y_scalar_get_instance_private(Y_SCALAR(src));
  YScalarPrivate *dpriv = y_scalar_get_instance_private(Y_SCALAR(dst));
  dpriv->value = priv->value;
  return Y_DATA (dst);
}

static double
y_scalar_val_get_value (YScalar *dat)
{
  YScalarPrivate *priv = y_scalar_get_instance_private(Y_SCALAR(dat));
  return priv->value;
}

static void
y_scalar_val_class_init (YScalarValClass *scalarval_klass)
{
  YDataClass *ydata_klass = (YDataClass *) scalarval_klass;
  YScalarClass *scalar_klass = (YScalarClass *) scalarval_klass;

  ydata_klass->dup = y_scalar_val_dup;
  scalar_klass->get_value = y_scalar_val_get_value;
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
  YScalarPrivate *priv = y_scalar_get_instance_private(Y_SCALAR(res));
  priv->value = val;

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
  YScalarPrivate *priv = y_scalar_get_instance_private(Y_SCALAR(s));
  return &priv->value;
}


/*************************************************************************/

/**
 * SECTION: y-vector
 * @short_description: Base class for one-dimensional array data objects.
 *
 * Abstract base class for data classes representing one dimensional arrays.
 */

typedef struct {
	int len;	/* negative if dirty, includes missing values */
	double *values;	/* NULL = inititialized/unsupported, nan = missing */
	double minimum, maximum;
} YVectorPrivate;

/**
 * YVector:
 *
 * Object representing a one-dimensional array of numbers.
 **/

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (YVector, y_vector, Y_TYPE_DATA);

static void
_data_array_emit_changed (YData *data)
{
	YDataPrivate *priv = y_data_get_instance_private(data);
	priv->flags &= ~(Y_DATA_CACHE_IS_VALID | Y_DATA_SIZE_CACHED | Y_DATA_HAS_VALUE | Y_DATA_MINMAX_CACHED);
}

static void
_data_vector_get_sizes (YData *data, unsigned int *sizes)
{
	YVector *vector = (YVector *) data;

	sizes[0] = y_vector_get_len (vector);
}

static void
_data_vector_get_bounds (YData *data, double *minimum, double *maximum)
{
	y_vector_get_minmax ((YVector *) data, minimum, maximum);
}

static char *
_vector_serialize (YData *dat, gpointer user)
{
	YVector *vec = (YVector *) dat;
  YVectorPrivate *vpriv = y_vector_get_instance_private(vec);
	GString *str;
	char sep;
	unsigned i;

	sep = '\t';
	str = g_string_new (NULL);

	for (i = 0; i < vpriv->len; i++) {
		char *s = render_val (vpriv->values[i]);
		if (i) g_string_append_c (str, sep);
		g_string_append (str, s);
		g_free (s);
	}
	return g_string_free (str, FALSE);
}

static void
y_vector_init (YVector *vec)
{
  YVectorPrivate *vpriv = y_vector_get_instance_private(vec);
  vpriv->len = 0;
  vpriv->values = NULL;
}

static void
y_vector_class_init (YVectorClass *vec_class)
{
  YDataClass *data_class = (YDataClass *) vec_class;
	data_class->emit_changed = 	_data_array_emit_changed;
	data_class->n_dimensions = 	1;
	data_class->get_sizes =		_data_vector_get_sizes;
	data_class->get_bounds =	_data_vector_get_bounds;
  data_class->serialize = _vector_serialize;
}

/**
 * y_vector_get_len :
 * @vec: #YVector
 *
 * Get the number of values in @vec and caches it.
 *
 * Returns: the length
 **/
unsigned int
y_vector_get_len (YVector *vec)
{
	g_return_val_if_fail (Y_IS_VECTOR(vec),0);
	YData *data = Y_DATA(vec);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YVectorPrivate *vpriv = y_vector_get_instance_private(vec);
	if (! (priv->flags & Y_DATA_SIZE_CACHED)) {
		YVectorClass const *klass = Y_VECTOR_GET_CLASS (vec);

		g_return_val_if_fail (klass != NULL, 0);

		vpriv->len = (*klass->load_len) (vec);
		priv->flags |= Y_DATA_SIZE_CACHED;
	}

	return vpriv->len;
}

/**
 * y_vector_get_values :
 * @vec: #YVector
 *
 * Get the array of values of @vec and cache them. 
 *
 * Returns: an array.
 **/
const double *
y_vector_get_values (YVector *vec)
{
  g_return_val_if_fail (Y_IS_VECTOR(vec),NULL);
  YData *data = Y_DATA(vec);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YVectorPrivate *vpriv = y_vector_get_instance_private(vec);
	if (! (priv->flags & Y_DATA_CACHE_IS_VALID)) {
		YVectorClass const *klass = Y_VECTOR_GET_CLASS (vec);

		g_return_val_if_fail (klass != NULL, NULL);

		double *v = (*klass->load_values) (vec);

	  vpriv->values = v;

	  priv->flags |= Y_DATA_CACHE_IS_VALID;
	}

	return vpriv->values;
}

/**
 * y_vector_get_value :
 * @vec: #YVector
 * @i: index
 *
 * Get a value in @vec.
 *
 * Returns: the value
 **/
double
y_vector_get_value (YVector *vec, unsigned i)
{
  g_assert(Y_IS_VECTOR(vec));
	YData *data = Y_DATA(vec);
	YDataPrivate *priv = y_data_get_instance_private(data);
  g_assert(priv);
	unsigned int len = y_vector_get_len(vec);
	g_return_val_if_fail (i < len, NAN);
	if (! (priv->flags & Y_DATA_CACHE_IS_VALID)) {
		YVectorClass const *klass = Y_VECTOR_GET_CLASS (vec);
    g_return_val_if_fail (klass != NULL, NAN);
		return (*klass->get_value) (vec, i);
	}
	YVectorPrivate *vpriv = y_vector_get_instance_private(vec);
	return vpriv->values [i];
}

/**
 * y_vector_get_str :
 * @vec: #YVector
 * @i: index
 * @format: a format string
 *
 * Get a string representation of an element in @vec.
 *
 * Returns: the string. The caller is
 * 	responsible for freeing it.
 **/
char *
y_vector_get_str (YVector *vec, unsigned int i, const gchar *format)
{
  char buf[G_ASCII_DTOSTR_BUF_SIZE];
  double val = y_vector_get_value(vec, i);
  g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, format, val);
  return g_strdup (buf);
}

static int
range_increasing (double const *xs, int n)
{
	int i = 0;
	double last;
	g_return_val_if_fail (n == 0 || xs != NULL, 0);
	while ( i < n && isnan (xs[i]))
		i++;
	if (i == n)
		return 0;
	last = xs[i];
	for (i = i + 1; i < n; i++) {
		if (isnan (xs[i]))
		    continue;
		if (last >= xs[i])
			return 0;
		last = xs[i];
	}
	return 1;
}

static int
range_decreasing (double const *xs, int n)
{
	int i = 0;
	double last;
	g_return_val_if_fail (n == 0 || xs != NULL, 0);
	while ( i < n &&  isnan (xs[i]))
		i++;
	if (i == n)
		return 0;
	last = xs[i];
	for (i = i + 1; i < n; i++) {
		if (isnan (xs[i]))
		    continue;
		if (last <= xs[i])
			return 0;
		last = xs[i];
	}
	return 1;
}

static int
range_vary_uniformly (double const *xs, int n)
{
	return range_increasing (xs, n) || range_decreasing (xs, n);
}

/**
 * y_vector_is_varying_uniformly :
 * @data: #YVector
 *
 * Returns whether elements of @data only increase or only decrease.
 *
 * Returns: %TRUE if elements of @data strictly increase or decrease.
 **/
gboolean
y_vector_is_varying_uniformly (YVector *data)
{
	double const *values;
	unsigned int n_values;

	g_return_val_if_fail (Y_IS_DATA (data), FALSE);

	values = y_vector_get_values (data);
	if (values == NULL)
		return FALSE;

	n_values = y_vector_get_len (data);
	if (n_values < 1)
		return FALSE;

	return range_vary_uniformly (values, n_values);
}

/**
 * y_vector_get_minmax :
 * @vec: #YVector
 * @min: (out)(nullable): return location for minimum value, or @NULL
 * @max: (out)(nullable): return location for maximum value, or @NULL
 *
 * Get the minimum and maximum values in @vec and cache them.
 **/
void
y_vector_get_minmax (YVector *vec, double *min, double *max)
{
  YData *data = Y_DATA(vec);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YVectorPrivate *vpriv = y_vector_get_instance_private(vec);

	if(!(priv->flags & Y_DATA_MINMAX_CACHED)) {
	  const double *v = y_vector_get_values (vec);
    if(v==NULL) return;

	  double minimum = DBL_MAX, maximum = -DBL_MAX;

	  unsigned int i = y_vector_get_len(vec);

	  while (i-- > 0) {
		  if (!isfinite (v[i]))
			  continue;
		  if (minimum > v[i])
			  minimum = v[i];
		  if (maximum < v[i])
			  maximum = v[i];
	  }
	  vpriv->minimum = minimum;
	  vpriv->maximum = maximum;
	  priv->flags |= Y_DATA_MINMAX_CACHED;
	  {
			if (isfinite (minimum) && isfinite (maximum) && minimum <= maximum)
				priv->flags |= Y_DATA_HAS_VALUE;
		}

	}

	if (min != NULL)
		*min = vpriv->minimum;
	if (max != NULL)
		*max = vpriv->maximum;
}

/**
 * y_vector_vary_uniformly :
 * @vec: #YVector
 *
 * Returns whether elements of @vec strictly increase or decrease with increasing index.
 *
 * Returns: %TRUE if elements of @vec only increase or decrease.
 **/
gboolean
y_vector_vary_uniformly (YVector *vec)
{
	const double *data = y_vector_get_values (vec);
	int length = y_vector_get_len (vec);
	return range_vary_uniformly (data, length);
}

/*************************************************************************/

/**
 * SECTION: y-matrix
 * @short_description: Base class for two-dimensional array data objects.
 *
 * Abstract base class for data classes representing two dimensional arrays.
 */

typedef struct {
	YMatrixSize size;	/* negative if dirty, includes missing values */
	double *values;	/* NULL = uninitialized/unsupported, nan = missing */
	double minimum, maximum;
} YMatrixPrivate;

/**
 * YMatrix:
 *
 * Object representing a two-dimensional array of numbers.
 **/

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (YMatrix, y_matrix, Y_TYPE_DATA);

static void
_data_matrix_get_sizes (YData *data, unsigned int *sizes)
{
	YMatrix *matrix = (YMatrix *) data;
	YMatrixSize size;

	size = y_matrix_get_size (matrix);

	sizes[0] = size.columns;
	sizes[1] = size.rows;
}

static void
_data_matrix_get_bounds (YData *data, double *minimum, double *maximum)
{
	y_matrix_get_minmax ((YMatrix *) data, minimum, maximum);
}

static char *
_matrix_serialize (YData *dat, gpointer user)
{
	YMatrix *mat = (YMatrix *) dat;
  YMatrixPrivate *mpriv = y_matrix_get_instance_private(mat);
	GString *str;
	size_t c, r;
	char col_sep = '\t';
	char row_sep = '\n';

	str = g_string_new (NULL);
	for (r = 0; r < mpriv->size.rows; r++) {
		if (r) g_string_append_c (str, row_sep);
		for (c = 0; c < mpriv->size.columns; c++) {
			double val = mpriv->values[r * mpriv->size.columns + c];
			char *s = render_val (val);
			if (c) g_string_append_c (str, col_sep);
			g_string_append (str, s);
			g_free (s);
		}
	}

	return g_string_free (str, FALSE);
}

static void
y_matrix_class_init (YMatrixClass *mat_class)
{
  YDataClass *data_class = Y_DATA_CLASS(mat_class);

	data_class->emit_changed = 	_data_array_emit_changed;
	data_class->n_dimensions = 	2;
	data_class->get_sizes =		_data_matrix_get_sizes;
	data_class->get_bounds =	_data_matrix_get_bounds;
  data_class->serialize = _matrix_serialize;
}

static void
y_matrix_init (YMatrix *mat) {}

/**
 * y_matrix_get_size: (skip)
 * @mat: #YMatrix
 *
 * Get the size of a #YMatrix.
 *
 * Returns: the matrix size
 **/
YMatrixSize
y_matrix_get_size (YMatrix *mat)
{
	static YMatrixSize null_size = {0, 0};
	if (!mat)
		return null_size;
	YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YMatrixPrivate *mpriv = y_matrix_get_instance_private(mat);
	if (! (priv->flags & Y_DATA_SIZE_CACHED)) {
		YMatrixClass const *klass = Y_MATRIX_GET_CLASS (mat);

		g_return_val_if_fail (klass != NULL, null_size);

		mpriv->size = (*klass->load_size) (mat);
		priv->flags |= Y_DATA_SIZE_CACHED;
	}

	return mpriv->size;
}

/**
 * y_matrix_get_rows:
 * @mat: #YMatrix
 *
 * Get the number of rows in a #YMatrix.
 *
 * Returns: the number of rows in @mat
 **/
unsigned int
y_matrix_get_rows (YMatrix *mat)
{
	if (!mat)
		return 0;
  YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YMatrixPrivate *mpriv = y_matrix_get_instance_private(mat);
	if (! (priv->flags & Y_DATA_SIZE_CACHED)) {
		YMatrixClass const *klass = Y_MATRIX_GET_CLASS (mat);

		g_return_val_if_fail (klass != NULL, 0);

		mpriv->size = (*klass->load_size) (mat);
		priv->flags |= Y_DATA_SIZE_CACHED;
	}

	return mpriv->size.rows;
}

/**
 * y_matrix_get_columns :
 * @mat: #YMatrix
 *
 * Get the number of columns in a #YMatrix.
 *
 * Returns: the number of columns in @mat
 **/
unsigned int
y_matrix_get_columns (YMatrix *mat)
{
	if (!mat)
		return 0;
	YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YMatrixPrivate *mpriv = y_matrix_get_instance_private(mat);
	if (! (priv->flags & Y_DATA_SIZE_CACHED)) {
		YMatrixClass const *klass = Y_MATRIX_GET_CLASS (mat);

		g_return_val_if_fail (klass != NULL, 0);

		mpriv->size = (*klass->load_size) (mat);
		priv->flags |= Y_DATA_SIZE_CACHED;
	}

	return mpriv->size.columns;
}

/**
 * y_matrix_get_values :
 * @mat: #YMatrix
 *
 * Get the array of values of @mat.
 *
 * Returns: an array.
 **/
const double *
y_matrix_get_values (YMatrix *mat)
{
  YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YMatrixPrivate *mpriv = y_matrix_get_instance_private(mat);
	if (! (priv->flags & Y_DATA_CACHE_IS_VALID)) {
		YMatrixClass const *klass = Y_MATRIX_GET_CLASS (mat);

		g_return_val_if_fail (klass != NULL, NULL);

		mpriv->values = (*klass->load_values) (mat);

    priv->flags |= Y_DATA_CACHE_IS_VALID;
	}

	return mpriv->values;
}

/**
 * y_matrix_get_value :
 * @mat: #YMatrix
 * @i: row
 * @j: column
 *
 * Get a value in @mat.
 *
 * Returns: the value
 **/
double
y_matrix_get_value (YMatrix *mat, unsigned i, unsigned j)
{
  YMatrixPrivate *mpriv = y_matrix_get_instance_private(mat);
	g_return_val_if_fail ((i < mpriv->size.rows) && (j < mpriv->size.columns), NAN);
	YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	if (! (priv->flags & Y_DATA_CACHE_IS_VALID)) {
		YMatrixClass const *klass = Y_MATRIX_GET_CLASS (mat);
		g_return_val_if_fail (klass != NULL, NAN);
		return (*klass->get_value) (mat, i, j);
	}

	return mpriv->values[i * mpriv->size.columns + j];
}

/**
 * y_matrix_get_str :
 * @mat: #YMatrix
 * @i: row
 * @j: column
 * @format: a format string
 *
 * Get a string representation of a value in @mat.
 *
 * Returns: the string
 **/
char *
y_matrix_get_str (YMatrix *mat, unsigned i, unsigned j, const gchar *format)
{
  char buf[G_ASCII_DTOSTR_BUF_SIZE];
  double val = y_matrix_get_value(mat, i, j);
  g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, format, val);
  return g_strdup (buf);
}

/**
 * y_matrix_get_minmax :
 * @mat: #YMatrix
 * @min: (out)(nullable): return location for minimum value, or @NULL
 * @max: (out)(nullable): return location for maximum value, or @NULL
 *
 * Get the minimum and maximum values in @mat.
 **/
void
y_matrix_get_minmax (YMatrix *mat, double *min, double *max)
{
  YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YMatrixPrivate *mpriv = y_matrix_get_instance_private(mat);
	if(!(priv->flags & Y_DATA_MINMAX_CACHED)) {
	  const double *v = y_matrix_get_values (mat);

	  double minimum = DBL_MAX, maximum = -DBL_MAX;

	  YMatrixSize s = y_matrix_get_size(mat);
	  int i = s.rows*s.columns;

	  while (i-- > 0) {
		  if (!isfinite (v[i]))
			  continue;
		  if (minimum > v[i])
			  minimum = v[i];
		  if (maximum < v[i])
			  maximum = v[i];
	  }
	  mpriv->minimum = minimum;
	  mpriv->maximum = maximum;
	  priv->flags |= Y_DATA_MINMAX_CACHED;
	  {
			if (isfinite (minimum) && isfinite (maximum) && minimum <= maximum)
				priv->flags |= Y_DATA_HAS_VALUE;
		}

	}

	if (min != NULL)
		*min = mpriv->minimum;
	if (max != NULL)
		*max = mpriv->maximum;
}

/**********************/

/**
 * SECTION: y-three-d-array
 * @short_description: Base class for three-dimensional array data objects.
 *
 * Abstract base class for data classes representing three dimensional arrays.
 */

typedef struct {
	YThreeDArraySize size;	/* negative if dirty, includes missing values */
	double *values;	/* NULL = uninitialized/unsupported, nan = missing */
	double minimum, maximum;
} YThreeDArrayPrivate;

/**
 * YThreeDArray:
 *
 * Object representing a three-dimensional array of numbers.
 **/

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (YThreeDArray, y_three_d_array, Y_TYPE_DATA);

static void
_data_tda_get_sizes (YData *data, unsigned int *sizes)
{
	YThreeDArray *matrix = (YThreeDArray *) data;
	YThreeDArraySize size;

	size = y_three_d_array_get_size (matrix);

	sizes[0] = size.columns;
	sizes[1] = size.rows;
	sizes[2] = size.layers;
}

static void
_data_tda_get_bounds (YData *data, double *minimum, double *maximum)
{
	y_three_d_array_get_minmax ((YThreeDArray *) data, minimum, maximum);
}

#if 0
static char *
_three_d_array_val_serialize (YData const *dat, gpointer user)
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
#endif

static void
y_three_d_array_class_init (YThreeDArrayClass *mat_class)
{
  YDataClass *data_class = Y_DATA_CLASS(mat_class);

	data_class->emit_changed = 	_data_array_emit_changed;
	data_class->n_dimensions = 	3;
	data_class->get_sizes =		_data_tda_get_sizes;
	data_class->get_bounds =	_data_tda_get_bounds;
}

static void
y_three_d_array_init (YThreeDArray *mat) {}

/**
 * y_three_d_array_get_size: (skip)
 * @mat: #YThreeDArray
 *
 * Get the size of a #YThreeDArray.
 *
 * Returns: the matrix size
 **/
YThreeDArraySize
y_three_d_array_get_size (YThreeDArray *mat)
{
	static YThreeDArraySize null_size = {0, 0, 0};
	if (!mat)
		return null_size;
	YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YThreeDArrayPrivate *mpriv = y_three_d_array_get_instance_private(mat);
	if (! (priv->flags & Y_DATA_SIZE_CACHED)) {
		YThreeDArrayClass const *klass = Y_THREE_D_ARRAY_GET_CLASS (mat);

		g_return_val_if_fail (klass != NULL, null_size);

		mpriv->size = (*klass->load_size) (mat);
		priv->flags |= Y_DATA_SIZE_CACHED;
	}

	return mpriv->size;
}

/**
 * y_three_d_array_get_rows:
 * @mat: #YThreeDArray
 *
 * Get the number of rows in a #YThreeDArray.
 *
 * Returns: the number of rows in @mat
 **/
unsigned int
y_three_d_array_get_rows (YThreeDArray *mat)
{
	if (!mat)
		return 0;
  YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YThreeDArrayPrivate *mpriv = y_three_d_array_get_instance_private(mat);
	if (! (priv->flags & Y_DATA_SIZE_CACHED)) {
		YThreeDArrayClass const *klass = Y_THREE_D_ARRAY_GET_CLASS (mat);

		g_return_val_if_fail (klass != NULL, 0);

		mpriv->size = (*klass->load_size) (mat);
		priv->flags |= Y_DATA_SIZE_CACHED;
	}

	return mpriv->size.rows;
}

/**
 * y_three_d_array_get_columns :
 * @mat: #YThreeDArray
 *
 * Get the number of columns in a #YThreeDArray.
 *
 * Returns: the number of columns in @mat
 **/
unsigned int
y_three_d_array_get_columns (YThreeDArray *mat)
{
	if (!mat)
		return 0;
	YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YThreeDArrayPrivate *mpriv = y_three_d_array_get_instance_private(mat);
	if (! (priv->flags & Y_DATA_SIZE_CACHED)) {
		YThreeDArrayClass const *klass = Y_THREE_D_ARRAY_GET_CLASS (mat);

		g_return_val_if_fail (klass != NULL, 0);

		mpriv->size = (*klass->load_size) (mat);
		priv->flags |= Y_DATA_SIZE_CACHED;
	}

	return mpriv->size.columns;
}

/**
 * y_three_d_array_get_layers :
 * @mat: #YThreeDArray
 *
 * Get the number of layers in a #YThreeDArray.
 *
 * Returns: the number of layers in @mat
 **/
unsigned int
y_three_d_array_get_layers (YThreeDArray *mat)
{
	if (!mat)
		return 0;
	YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YThreeDArrayPrivate *mpriv = y_three_d_array_get_instance_private(mat);
	if (! (priv->flags & Y_DATA_SIZE_CACHED)) {
		YThreeDArrayClass const *klass = Y_THREE_D_ARRAY_GET_CLASS (mat);

		g_return_val_if_fail (klass != NULL, 0);

		mpriv->size = (*klass->load_size) (mat);
		priv->flags |= Y_DATA_SIZE_CACHED;
	}

	return mpriv->size.layers;
}

/**
 * y_three_d_array_get_values :
 * @mat: #YThreeDArray
 *
 * Get the array of values of @mat.
 *
 * Returns: an array.
 **/
const double *
y_three_d_array_get_values (YThreeDArray *mat)
{
  YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YThreeDArrayPrivate *mpriv = y_three_d_array_get_instance_private(mat);
	if (! (priv->flags & Y_DATA_CACHE_IS_VALID)) {
		YThreeDArrayClass const *klass = Y_THREE_D_ARRAY_GET_CLASS (mat);

		g_return_val_if_fail (klass != NULL, NULL);

		mpriv->values = (*klass->load_values) (mat);

    priv->flags |= Y_DATA_CACHE_IS_VALID;
	}

	return mpriv->values;
}

/**
 * y_three_d_array_get_value :
 * @mat: #YThreeDArray
 * @i: layer
 * @j: row
 * @k: column
 *
 * Get a value in @mat.
 *
 * Returns: the value
 **/
double
y_three_d_array_get_value (YThreeDArray *mat, unsigned i, unsigned j, unsigned k)
{
  YThreeDArrayPrivate *mpriv = y_three_d_array_get_instance_private(mat);
	g_return_val_if_fail ((i < mpriv->size.rows) && (j < mpriv->size.columns) && (k < mpriv->size.layers), NAN);
	YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	if (! (priv->flags & Y_DATA_CACHE_IS_VALID)) {
		YThreeDArrayClass const *klass = Y_THREE_D_ARRAY_GET_CLASS (mat);
		g_return_val_if_fail (klass != NULL, NAN);
		return (*klass->get_value) (mat, i, j, k);
	}

	return mpriv->values[i * mpriv->size.rows*mpriv->size.columns + j*mpriv->size.columns + k];
}

/**
 * y_three_d_array_get_str :
 * @mat: #YThreeDArray
 * @i: row
 * @j: column
 * @k: layer
 * @format: a format string
 *
 * Get a string representation of a value in @mat.
 *
 * Returns: the string
 **/
char *
y_three_d_array_get_str (YThreeDArray *mat, unsigned i, unsigned j, unsigned k, const gchar *format)
{
  char buf[G_ASCII_DTOSTR_BUF_SIZE];
  double val = y_three_d_array_get_value(mat, i, j, k);
  g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, format, val);
  return g_strdup (buf);
}

/**
 * y_three_d_array_get_minmax :
 * @mat: #YThreeDArray
 * @min: (out)(nullable): return location for minimum value, or @NULL
 * @max: (out)(nullable): return location for maximum value, or @NULL
 *
 * Get the minimum and maximum values in @mat.
 **/
void
y_three_d_array_get_minmax (YThreeDArray *mat, double *min, double *max)
{
  YData *data = Y_DATA(mat);
	YDataPrivate *priv = y_data_get_instance_private(data);
	YThreeDArrayPrivate *mpriv = y_three_d_array_get_instance_private(mat);
	if(!(priv->flags & Y_DATA_MINMAX_CACHED)) {
	  const double *v = y_three_d_array_get_values (mat);

	  double minimum = DBL_MAX, maximum = -DBL_MAX;

	  YThreeDArraySize s = y_three_d_array_get_size(mat);
	  int i = s.rows*s.columns*s.layers;

	  while (i-- > 0) {
		  if (!isfinite (v[i]))
			  continue;
		  if (minimum > v[i])
			  minimum = v[i];
		  if (maximum < v[i])
			  maximum = v[i];
	  }
	  mpriv->minimum = minimum;
	  mpriv->maximum = maximum;
	  priv->flags |= Y_DATA_MINMAX_CACHED;
	  {
			if (isfinite (minimum) && isfinite (maximum) && minimum <= maximum)
				priv->flags |= Y_DATA_HAS_VALUE;
		}

	}

	if (min != NULL)
		*min = mpriv->minimum;
	if (max != NULL)
		*max = mpriv->maximum;
}

/**********************/

/**
 * SECTION: y-struct
 * @short_description: A dictionary containing data objects.
 *
 * A data object that can contain other data objects.
 */

struct _YStruct {
	YData	 base;
	GHashTable *hash;
};

/**
 * YStruct:
 *
 * Object representing a dictionary full of YData objects.
 **/

G_DEFINE_TYPE (YStruct, y_struct, Y_TYPE_DATA);

static void
y_struct_finalize (GObject *obj)
{
	YStruct *s = (YStruct *)obj;
  	g_hash_table_unref(s->hash);

  	GObjectClass *obj_class = G_OBJECT_CLASS(y_struct_parent_class);

	(*obj_class->finalize) (obj);
}

static void
y_struct_class_init (YStructClass *val_klass)
{
	YDataClass *ydata_klass = (YDataClass *) val_klass;
	GObjectClass *gobject_klass = (GObjectClass *) val_klass;

  gobject_klass->finalize = y_struct_finalize;
  ydata_klass->n_dimensions = -1;
	//ydata_klass->dup	= y_vector_val_dup;
}

static void
y_struct_init(YStruct *s) {
  s->hash = g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_object_unref);
}

/**
 * y_struct_get_data :
 * @s: #YStruct
 * @name: string id
 *
 * Get a data object from the struct.
 *
 * Returns: (transfer none): the data.
 **/
YData *y_struct_get_data(YStruct     *s,
                         const gchar *name)
{
  return g_hash_table_lookup(s->hash,name);
}

/**
 * y_struct_set_data :
 * @s: #YStruct
 * @name: string id
 * @d: (transfer full): #YData
 *
 * Set a data object.
 **/
void y_struct_set_data(YStruct     *s,
                         const gchar *name, YData *d)
{
  g_hash_table_insert(s->hash,g_strdup(name),g_object_ref_sink(d));
}

/**
 * y_struct_foreach :
 * @s: #YStruct
 * @f: (scope call): #GHFunc
 * @user_data: user data
 *
 * Set a data object.
 **/
void y_struct_foreach(YStruct   *s,
                      GHFunc  f,
                      gpointer   user_data)
{
  g_hash_table_foreach(s->hash,f,user_data);
}
