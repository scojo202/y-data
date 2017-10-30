/*
 * y-fft-operation.c :
 *
 * Copyright (C) 2017 Scott O. Johnson (scojo202@gmail.com)
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
#include <complex.h>
#include "y-fft-operation.h"

/**
 * SECTION: y-fft-operation
 * @short_description: Operations that take Fourier transform of input.
 *
 *
 *
 *
 */

enum {
  FFT_PROP_0,
  FFT_PROP_TYPE
};

struct _YFFTOperation {
  YOperation	 base;
  guchar           type;
};

G_DEFINE_TYPE (YFFTOperation, y_fft_operation, Y_TYPE_OPERATION);

static void
y_fft_operation_set_property (GObject *gobject, guint param_id,
                                 GValue const *value, GParamSpec *pspec)
{
	YFFTOperation *sop = Y_FFT_OPERATION (gobject);

	switch (param_id) {
	case FFT_PROP_TYPE:
		sop->type = g_value_get_int (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, param_id, pspec);
		return; /* NOTE : RETURN */
	}
}

static void
y_fft_operation_get_property (GObject *gobject, guint param_id,
                                 GValue *value, GParamSpec *pspec)
{
	YFFTOperation *sop = Y_FFT_OPERATION (gobject);

	switch (param_id) {
	case FFT_PROP_TYPE:
		g_value_set_int (value, sop->type);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, param_id, pspec);
		return; /* NOTE : RETURN */
	}
}

static
int vector_fft_size (YOperation *op, YData *input, unsigned int *dims)
{
  int n_dims;
  g_assert(Y_IS_VECTOR(input));
  g_assert(dims);
  YVector *mat = Y_VECTOR(input);
  dims[0]=y_vector_get_len(mat)/2+1;
  n_dims=1;
  return n_dims;
}

typedef struct {
  YFFTOperation sop;
  double *input;
  unsigned int len;
  fftw_complex *inter;
  double *output;
  unsigned int out_len;
  fftw_plan plan;
} FFTOpData;

static
gpointer vector_fft_op_create_data(YOperation *op, gpointer data, YData *input)
{
  if(input==NULL) return NULL;
  FFTOpData *d;
  gboolean neu = TRUE;
  if(data==NULL) {
    d = g_new0(FFTOpData,1);
  } else {
    neu = FALSE;
    d=(FFTOpData *) data;
  }
  YFFTOperation *sop = Y_FFT_OPERATION(op);
  d->sop = *sop;
  YVector *vec = Y_VECTOR(input);
  unsigned int old_len = d->len;
  d->len = y_vector_get_len(vec);
  if(!neu) {
    if(old_len != d->len) {
      fftw_free(d->input);
      d->input = fftw_malloc(sizeof(double)*d->len);
    }
  }
  else {
    d->input = fftw_malloc(sizeof(double)*d->len);
  }
  unsigned int dims[1];
  vector_fft_size(op,input,dims);
  if(d->out_len != dims[0]) {
    if(d->inter)
      fftw_free(d->inter);
    if(d->output)
      g_free(d->output);
    d->out_len = dims[0];
    d->inter = fftw_malloc(sizeof(fftw_complex)*d->out_len);
    d->output = g_new(double,d->out_len);
  }
  g_assert(d->input);
  g_assert(d->inter);
  g_assert(d->len>0);
  d->plan = fftw_plan_dft_r2c_1d(d->len, d->input, d->inter, FFTW_ESTIMATE);
  memcpy(d->input,y_vector_get_values(vec),d->len*sizeof(double));
  return d;
}

static
void vector_fft_op_data_free(gpointer d)
{
  FFTOpData *s = (FFTOpData *) d;
  g_message("free");
  fftw_free(s->input);
  fftw_free(s->inter);
  g_free(s->output);
  g_free(d);
}

static
gpointer vector_fft_op(gpointer input)
{
  FFTOpData *d = (FFTOpData *) input;

  if(d==NULL)
    return NULL;

  //g_message("task data: index %d, width %d, type %u, input %p, nrow %u, ncol %u",d->index,d->width,d->type,d->input,d->nrow,d->ncol);

  if(d->sop.type == FFT_MAG || d->sop.type == FFT_PHASE) {
    fftw_execute(d->plan);
    int i;
    for(i=0;i<d->out_len;i++) {
      complex double ci = (complex double) d->inter[i];
      d->output[i]= (d->sop.type == FFT_MAG) ? cabs(ci) : carg(ci);
    }
  }
  return d->output;
}

static void
y_fft_operation_class_init (YFFTOperationClass *slice_klass)
{
  GObjectClass *gobject_klass = (GObjectClass *) slice_klass;
  gobject_klass->set_property = y_fft_operation_set_property;
  gobject_klass->get_property = y_fft_operation_get_property;
  YOperationClass *op_klass = (YOperationClass *) slice_klass;
  op_klass->thread_safe = FALSE;
  op_klass->op_size = vector_fft_size;
  op_klass->op_func = vector_fft_op;
  op_klass->op_data = vector_fft_op_create_data;
  op_klass->op_data_free = vector_fft_op_data_free;

  g_object_class_install_property (gobject_klass, FFT_PROP_TYPE,
		g_param_spec_int ("type", "Type",
			"Type of FFT operation",
			FFT_MAG, FFT_PHASE, FFT_MAG,
			G_PARAM_READWRITE));
}

static void
y_fft_operation_init(YFFTOperation *fft) {
  g_assert(Y_IS_FFT_OPERATION(fft));
  fft->type = FFT_MAG;
}

YOperation *y_fft_operation_new (int type)
{
  YOperation *o = g_object_new(Y_TYPE_FFT_OPERATION,"type",type,NULL);

  return o;
}
