/*
 * y-vector-ring.c :
 *
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
 
#include "y-vector-ring.h"
#include <math.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>

/**
 * SECTION: y-vector-ring
 * @short_description: A vector that grows as scalar values are added, up to a
 * maximum length, after which it throws away the first elements to make room
 * for more.
 *
 * Data class #YVectorRing
 *
 */

struct _YVectorRing {
	YVector	 base;
	unsigned	 n;
	unsigned int nmax;
	double *val;
	YScalar *source;
	gulong handler;
};

G_DEFINE_TYPE (YVectorRing, y_vector_ring, Y_TYPE_VECTOR);

static void
y_vector_ring_finalize (GObject *obj)
{
	YVectorRing *vec = (YVectorRing *)obj;
	if (vec->val)
		g_free(vec->val);
        if (vec->source) {
                g_object_unref(vec->source);
                g_signal_handler_disconnect(vec->source,vec->handler);
        }

	GObjectClass *obj_class = G_OBJECT_CLASS(y_vector_ring_parent_class);

	(*obj_class->finalize) (obj);
}

static YData *
y_vector_ring_dup (YData *src)
{
	YVectorRing *dst = g_object_new (G_OBJECT_TYPE (src), NULL);
	YVectorRing const *src_val = (YVectorRing const *)src;
	dst->val = g_new (double, src_val->n);
	memcpy (dst->val, src_val->val, src_val->n * sizeof (double));
	dst->n = src_val->n;
	return Y_DATA (dst);
}

static unsigned int
y_vector_ring_load_len (YVector *vec)
{
	return ((YVectorRing *)vec)->n;
}

static double *
y_vector_ring_load_values (YVector *vec)
{
	YVectorRing const *val = (YVectorRing const *)vec;

	return val->val;
}

static double
y_vector_ring_get_value (YVector *vec, unsigned i)
{
	YVectorRing const *val = (YVectorRing const *)vec;
	g_return_val_if_fail (val != NULL && val->val != NULL && i < val->n, NAN);
	return val->val[i];
}

static char *
render_val (double val)
{
		char buf[G_ASCII_DTOSTR_BUF_SIZE];
		g_ascii_dtostr (buf, G_ASCII_DTOSTR_BUF_SIZE, val);
		return g_strdup (buf);
}

static char *
y_vector_ring_serialize (YData *dat, gpointer user)
{
	YVectorRing *vec = Y_VECTOR_RING (dat);
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
y_vector_ring_unserialize (YData *dat, char const *str, gpointer user)
{
	YVectorRing *vec = Y_VECTOR_RING (dat);
	char sep, *end = (char*) str;
	double val;
	GArray *values;

	g_return_val_if_fail (str != NULL, TRUE);

	if (vec->val)
		g_free(vec->val);

	values = g_array_sized_new (FALSE, FALSE, sizeof(double), 16);
	sep = 0;
	vec->val = NULL;
	vec->n = 0;
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
y_vector_ring_class_init (YVectorRingClass *val_klass)
{
	YDataClass *ydata_klass = (YDataClass *) val_klass;
	YVectorClass *vector_klass = (YVectorClass *) val_klass;
	GObjectClass *gobject_klass = (GObjectClass *) val_klass;

	gobject_klass->finalize = y_vector_ring_finalize;
	ydata_klass->dup	= y_vector_ring_dup;
	ydata_klass->serialize	= y_vector_ring_serialize;
	ydata_klass->unserialize	= y_vector_ring_unserialize;
	vector_klass->load_len    = y_vector_ring_load_len;
	vector_klass->load_values = y_vector_ring_load_values;
	vector_klass->get_value   = y_vector_ring_get_value;
}

static void
y_vector_ring_init(YVectorRing *val) {}

/**
 * y_vector_ring_new:
 * @nmax: maximum length of array
 * @n: initial length of array
 *
 * If @n is not zero, elements are initialized to zero.
 *
 * Returns: a #YData
 *
 **/
YData *
y_vector_ring_new (unsigned nmax, unsigned n)
{
	YVectorRing *res = g_object_new (Y_TYPE_VECTOR_RING, NULL);
	res->val = g_new0(double, nmax);
	res->n = n;
	res->nmax = nmax;
	return Y_DATA (res);
}

/**
 * y_vector_ring_append :
 * @d: #YVectorRing
 * @val: new value
 *
 * Append a new value to the vector.
 *
 **/
void y_vector_ring_append(YVectorRing *d, double val)
{
  g_assert(Y_IS_VECTOR_RING(d));
  unsigned int l = MIN(d->nmax,y_vector_get_len(Y_VECTOR(d)));
  double *frames = d->val;
  if(l<d->nmax) {
    frames[l]=val;
    y_vector_ring_set_length(d, l+1);
  }
  else if (l==d->nmax) {
    memmove(frames, &frames[1], (l-1)*sizeof(double));
    frames[l-1]=val;
  }
  else return;
  y_data_emit_changed(Y_DATA(d));
}

/**
 * y_vector_ring_append_array :
 * @d: #YVectorRing
 * @arr: array
 * @len: array length
 *
 * Append a new value to the vector.
 *
 **/
void y_vector_ring_append_array(YVectorRing *d, double *arr, int len)
{
  g_assert(Y_IS_VECTOR_RING(d));
  g_assert(arr);
  unsigned int l = MIN(d->nmax,y_vector_get_len(Y_VECTOR(d)));
  double *frames = d->val;
  int i;
  if(l+len<d->nmax) {
    for(i=0;i<len;i++) {
      frames[i+l]=arr[i];
    }
    y_vector_ring_set_length(d, l+len);
  }
  /*else {
    memmove(frames, &frames[1], (l-1)*sizeof(double));
    frames[l-1]=val;
  }*/
  else return;
  y_data_emit_changed(Y_DATA(d));
}

static void
on_source_changed(YData *data, gpointer   user_data) {
        YVectorRing *d = Y_VECTOR_RING(user_data);
        YScalar *source = Y_SCALAR(data);
        y_vector_ring_append(d,y_scalar_get_value(source));
}

/**
 * y_vector_ring_set_source :
 * @d: #YVectorRing
 * @source: a #YScalar
 *
 * Set a source for the #YVectorRing. When the source emits a "changed" signal,
 * a new value will be appended to the vector.
 **/
void y_vector_ring_set_source(YVectorRing *d, YScalar *source)
{
        g_assert(Y_IS_VECTOR_RING(d));
        g_assert(Y_IS_SCALAR(source));
        if(d->source) {
                g_object_unref(d->source);
                g_signal_handler_disconnect(d->source,d->handler);
        }
        if(Y_IS_SCALAR(source)) {
                d->source = g_object_ref_sink(source);
        }
        else if(source==NULL) {
                d->source = NULL;
                return;
        }
        d->handler = g_signal_connect_after(source,"changed",G_CALLBACK(on_source_changed),d);
}

/**
 * y_vector_ring_set_length :
 * @d: #YVectorRing
 * @newlength: new length of array
 *
 * Set the current length of the #YVectorRing to a new value. If the new
 * length is longer than the previous length, tailing elements are set to
 * zero.
 **/
void y_vector_ring_set_length(YVectorRing *d, unsigned newlength)
{
  g_assert(Y_IS_VECTOR_RING(d));
  if(newlength<=d->nmax) {
    d->n = newlength;
    y_data_emit_changed(Y_DATA(d));
  }
}

