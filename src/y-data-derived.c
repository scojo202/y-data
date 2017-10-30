/*
 * y-data-derived.c :
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

#include <memory.h>
#include <math.h>
#include "y-data-derived.h"

/**
 * SECTION: y-data-derived
 * @short_description: Data objects that reflect the outputs of operations.
 *
 * 
 *
 * 
 */

struct _YVectorDerived {
  YVector	   base;
  YOperation     *op;
  unsigned int   currlen;
  YData          *input;
  double         *cache;
  gboolean       autorun;
  gboolean       running; /* is operation currently running? */
  gpointer       task_data;
  /* might need access to whether cache is OK */
};

G_DEFINE_TYPE (YVectorDerived, y_vector_derived, Y_TYPE_VECTOR);

static void
vector_derived_finalize (GObject *obj)
{
	YVectorDerived *vec = (YVectorDerived *)obj;
	/* unref matrix */
	g_object_unref(vec->input);
        if(vec->task_data) {
	  YOperationClass *klass = (YOperationClass *) G_OBJECT_GET_CLASS(vec->op);
	  if(klass->op_data_free) {
	    klass->op_data_free(vec->task_data);
	  }
	}
        if(vec->op) {
	  g_object_unref(vec->op);
	}

	GObjectClass *obj_class = G_OBJECT_CLASS(y_vector_derived_parent_class);

	obj_class->finalize (obj);
}

#if 0
static YData *
vector_derived_dup (YData const *src)
{
	YVectorDerived *dst = g_object_new (G_OBJECT_TYPE (src), NULL);
	YVectorDerived const *src_val = (YVectorDerived const *)src;
	return Y_DATA (dst);
}
#endif

static unsigned int
vector_derived_load_len (YVector *vec)
{
  YVectorDerived *vecd = (YVectorDerived *)vec;
  g_assert(vecd->op);
  YOperationClass *klass = (YOperationClass *) G_OBJECT_GET_CLASS(vecd->op);
  g_assert(klass);

  unsigned int newdim;
  g_assert(klass->op_size);
  if(vecd->input) {
    int ndims = klass->op_size(vecd->op,vecd->input,&newdim);
    g_assert(ndims==1);
  }
  else
    newdim = 0;

  return newdim;
}

static double *
vector_derived_load_values (YVector *vec)
{
  YVectorDerived *vecs = (YVectorDerived *)vec;

  double *v = NULL;

  unsigned int len = y_vector_get_len(vec);

  //g_message("load values, len is %u",len);

  if(vecs->currlen != len) {
    if(vecs->cache)
      g_free(vecs->cache);
    v = g_new0(double,len);
    vecs->currlen=len;
    vecs->cache = v;
  }
  else {
    v = vecs->cache;
  }
  if(v==NULL) return NULL;

  /* call op */
  YOperationClass *klass = Y_OPERATION_GET_CLASS(vecs->op);
  if(vecs->task_data == NULL) {
    vecs->task_data = y_operation_create_task_data(vecs->op,vecs->input);
  }
  else {
    y_operation_update_task_data(vecs->op,vecs->task_data,vecs->input);
  }
  double *dout = klass->op_func(vecs->task_data);
  if(dout==NULL) return NULL;
  memcpy(v,dout,len*sizeof(double));

  return v;
}

static double
vector_derived_get_value (YVector *vec, unsigned i)
{
  const double *d = y_vector_get_values(vec); /* fills the cache */
	return d[i];
}

#if 0
/*static char *
data_vector_slice_get_str (GODataVector *vec, unsigned i)
{
	YVectorSlice const *val = (YVectorSlice const *)vec;
	GOFormat const *fmt = NULL;

	g_return_val_if_fail (val != NULL && val->val != NULL && i < val->n, NULL);

	return render_val (val->val[i], fmt);
}
*/
#endif

static void
y_vector_derived_class_init (YVectorDerivedClass *slice_klass)
{
  GObjectClass *gobject_klass = (GObjectClass *) slice_klass;
  YDataClass *ydata_klass = (YDataClass *) gobject_klass;
  YVectorClass *vector_klass = (YVectorClass *) gobject_klass;

	gobject_klass->finalize = vector_derived_finalize;
	//ydata_klass->dup	= data_vector_slice_dup;
	//ydata_klass->serialize	= data_vector_slice_serialize;
	vector_klass->load_len    = vector_derived_load_len;
	vector_klass->load_values = vector_derived_load_values;
	vector_klass->get_value   = vector_derived_get_value;
	//vector_klass->get_str     = data_vector_slice_get_str;
}

static void
y_vector_derived_init(YVectorDerived *der) {
  der->input = NULL;
  der->cache = NULL;
}

static void
op_cb (GObject *source_object,
                        GAsyncResult *res,
                        gpointer user_data)
{
  /* set outputs */
  GTask *task = G_TASK(res);
  g_task_propagate_pointer(task,NULL);
  YVectorDerived *d = (YVectorDerived *) user_data;
  d->running = FALSE;
  y_data_emit_changed(Y_DATA(user_data));
}

static void
on_input_changed_after(YData *data, gpointer   user_data) {
  YVectorDerived *d = Y_VECTOR_DERIVED( user_data);
  /* if shape changed, adjust length */
  /* FIXME: this just loads the length every time */
  vector_derived_load_len(Y_VECTOR(d));
  if(!d->autorun) {
    y_data_emit_changed(Y_DATA(d));
  }
  else {
    if(d->running)
      return;
    d->running = TRUE;
    YOperationClass *klass = Y_OPERATION_GET_CLASS(d->op);
    if(klass->thread_safe) {
      /* get task data, run in a thread */
      y_operation_update_task_data(d->op,d->task_data,data);
      y_operation_run_task(d->op,d->task_data,op_cb,d);
    }
    else {
      /* load new values into the cache */
      vector_derived_load_values(Y_VECTOR(d));
      d->running=FALSE;
      y_data_emit_changed(Y_DATA(d));
    }
  }
}

static void
on_op_changed(GObject *gobject, GParamSpec *pspec, gpointer user_data)
{
  YVectorDerived *d = Y_VECTOR_DERIVED( user_data);
  vector_derived_load_len(Y_VECTOR(d));
  y_data_emit_changed(Y_DATA(d));
}

void y_vector_derived_set_input (YVectorDerived *der,
                                   YData    *d)
{
  g_assert(Y_IS_VECTOR_DERIVED(der));
  g_assert(Y_IS_DATA(d));
  der->input = g_object_ref_sink(d);
  der->currlen=0;
  g_signal_connect_after(d,"changed",G_CALLBACK(on_input_changed_after),der);
  y_data_emit_changed(Y_DATA(der));
}

YData	*y_vector_derived_new (YData *m, YOperation *op)
{
  if(m)
    g_assert(Y_IS_DATA(m));
  g_assert(Y_IS_OPERATION(op));

  YData *d = g_object_new(Y_TYPE_VECTOR_DERIVED,NULL);

  YVectorDerived *vd = (YVectorDerived*) d;
  vd->op = op;
  g_object_ref(vd->op);

  /* listen to "notify" from op for property changes */
  g_signal_connect(op,"notify",G_CALLBACK(on_op_changed),vd);

  if(Y_IS_DATA(d) && Y_IS_DATA(m)) {
    y_vector_derived_set_input(vd,m);
  }
  return d;
}

void y_vector_derived_set_autorun (YVectorDerived *dvs,
                                   gboolean        autorun)
{
  g_assert(Y_IS_VECTOR_DERIVED(dvs));
  dvs->autorun = autorun;
}
