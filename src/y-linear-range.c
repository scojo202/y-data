/*
 * y-linear-range.c :
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

#include "y-linear-range.h"
#include <math.h>

/* make v0 and dv properties?
 *
 * */

/**
 * SECTION: y-linear-range
 * @short_description: Vector for equally spaced data.
 *
 * Data for equally spaced data.
 */

struct _YLinearRangeVector {
  YVector	 base;
  double v0;
  double dv;
  unsigned n;
  double *values;
};

G_DEFINE_TYPE (YLinearRangeVector, y_linear_range_vector, Y_TYPE_VECTOR);

static GObjectClass *vector_parent_klass;

static void
_linear_range_vector_get_bounds (YData *data, double *minimum, double *maximum)
{
  YLinearRangeVector const *v = (YLinearRangeVector const *)data;
  if(minimum!=NULL) {
    *minimum=(v->dv>0) ? v->v0 : v->v0+v->dv*(v->n-1);
  }
  if(maximum!=NULL) {
    *maximum=(v->dv>0) ? v->v0+v->dv*(v->n-1) : v->v0;
  }
}

static void
data_vector_val_finalize (GObject *obj)
{
	YLinearRangeVector *vec = (YLinearRangeVector *)obj;
	
	g_free(vec->values);

	(*vector_parent_klass->finalize) (obj);
}

static YData *
data_vector_val_dup (YData *src)
{
	YLinearRangeVector *dst = g_object_new (G_OBJECT_TYPE (src), NULL);
	YLinearRangeVector const *src_val = (YLinearRangeVector const *)src;
	dst->v0 = src_val->v0;
	dst->dv = src_val->dv;
	dst->n = src_val->n;
	return Y_DATA (dst);
}

static unsigned int
data_vector_val_load_len (YVector *vec)
{
	return ((YLinearRangeVector *)vec)->n;
}

#define get_val(d,i) (d->v0+i*(d->dv))

static double *
data_vector_val_load_values (YVector *vec)
{
	YLinearRangeVector *val = (YLinearRangeVector *)vec;
	int i = val->n;
	
  val->values = g_new0(double, val->n);

	if(y_vector_get_len(vec) != val->n) {
	  data_vector_val_load_len(vec);
	}
  g_assert(isfinite(val->v0));
  g_assert(isfinite(val->dv));

	while (i-- > 0) {
	  val->values[i]=get_val(val,i);
	}
	return val->values;
}

static double
data_vector_val_get_value (YVector *vec, unsigned i)
{
	YLinearRangeVector const *val = (YLinearRangeVector const *)vec;
	g_return_val_if_fail (val != NULL && i < val->n, NAN);
	return get_val(val,i);
}

static void
y_linear_range_vector_init(YLinearRangeVector *v) {}

static void
y_linear_range_vector_class_init (YLinearRangeVectorClass *klass)
{
  GObjectClass *gobject_klass = (GObjectClass *) klass;
	YDataClass *ydata_klass = (YDataClass *) klass;
	YVectorClass *vector_klass = (YVectorClass *) klass;

	vector_parent_klass = g_type_class_peek_parent (gobject_klass);
	gobject_klass->finalize = data_vector_val_finalize;
	ydata_klass->dup	= data_vector_val_dup;
	//ydata_klass->get_bounds =	_linear_range_vector_get_bounds;
	//ydata_klass->serialize	= data_vector_val_serialize;
	vector_klass->load_len    = data_vector_val_load_len;
	vector_klass->load_values = data_vector_val_load_values;
	vector_klass->get_value   = data_vector_val_get_value;
}

/**
 * y_linear_range_vector_set_length :
 * @d: a #YLinearRangeVector
 * @newlength: length
 *
 * Set the length of @d. 
 *
 **/

void y_linear_range_vector_set_length(YLinearRangeVector *d, unsigned newlength)
{
  g_assert(Y_IS_LINEAR_RANGE_VECTOR(d));

  if(newlength!=d->n) {
    d->n = newlength;
    y_data_emit_changed(Y_DATA(d));
  }
}

/**
 * y_linear_range_vector_set_pars :
 * @d: a #YLinearRangeVector
 * @v0: first value
 * @dv: step size
 *
 * Set the initial value and step size of @d.
 *
 **/

void y_linear_range_vector_set_pars(YLinearRangeVector *d, double v0, double dv)
{
  g_assert(Y_IS_LINEAR_RANGE_VECTOR(d));
  d->v0 = v0;
  d->dv = dv;
  y_data_emit_changed(Y_DATA(d));
}

/**
 * y_linear_range_vector_new :
 * @v0: first value
 * @dv: step size
 * @n: length
 *
 * Create a new #YLinearRangeVector.
 *
 * Returns: a new #YLinearRangeVector as a #YData
 **/

YData *
y_linear_range_vector_new (double v0, double dv, unsigned n)
{
	YLinearRangeVector *res = g_object_new (Y_TYPE_LINEAR_RANGE_VECTOR, NULL);
	res->v0 = v0;
	res->dv = dv;
	res->n = n;
	return Y_DATA (res);
}

