/*
 * y-data-vector-slice.c :
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
#include "y-data.h"
#include "y-data-vector-slice.h"

/**
 * SECTION: y-operation-slice
 * @short_description: Operation that slices matrices, outputing vectors or matrices.
 *
 * 
 *
 * 
 */

enum {
  SLICE_PROP_0,
  SLICE_PROP_INDEX,
  SLICE_PROP_TYPE,
  SLICE_PROP_WIDTH,
  SLICE_PROP_MEAN
};

struct _YSliceOperation {
  YOperation	 base;
  int 		 index;
  guchar           type;
  int              width;
  int      index2;
  int      width2;
  gboolean   mean;
};

G_DEFINE_TYPE (YSliceOperation, y_slice_operation, Y_TYPE_OPERATION);

static void
y_slice_operation_set_property (GObject *gobject, guint param_id,
                                 GValue const *value, GParamSpec *pspec)
{
	YSliceOperation *sop = Y_SLICE_OPERATION (gobject);

	switch (param_id) {
	case SLICE_PROP_INDEX:
		sop->index = g_value_get_int (value);
		break;
	case SLICE_PROP_TYPE:
		sop->type = g_value_get_int (value);
		break;
  case SLICE_PROP_WIDTH:
		sop->width = g_value_get_int (value);
		break;
  case SLICE_PROP_MEAN:
		sop->mean = g_value_get_boolean (value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, param_id, pspec);
		return; /* NOTE : RETURN */
	}
}

static void
y_slice_operation_get_property (GObject *gobject, guint param_id,
                                 GValue *value, GParamSpec *pspec)
{
	YSliceOperation *sop = Y_SLICE_OPERATION (gobject);

	switch (param_id) {
	case SLICE_PROP_INDEX:
		g_value_set_int (value, sop->index);
		break;
	case SLICE_PROP_TYPE:
		g_value_set_int (value, sop->type);
		break;
	case SLICE_PROP_WIDTH:
		g_value_set_int (value, sop->width);
		break;
  case SLICE_PROP_MEAN:
		g_value_set_boolean (value, sop->mean);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, param_id, pspec);
		return; /* NOTE : RETURN */
	}
}

static
int vector_slice_size (YOperation *op, YData *input, unsigned int *dims)
{
  int n_dims;
  g_assert(Y_IS_MATRIX(input));
  g_assert(dims);
  YSliceOperation *sop = Y_SLICE_OPERATION(op);
  YMatrix *mat = Y_MATRIX(input);

  if((sop->type == SLICE_ROW) || (sop->type == SLICE_SUMROWS)) {
    dims[0] = y_matrix_get_columns(mat);
    dims[1] = 1;
    n_dims=1;
  }
  else if ((sop->type == SLICE_COL) || (sop->type == SLICE_SUMCOLS)) {
    dims[0] = y_matrix_get_rows(mat);
    dims[1] = 1;
    n_dims=1;
  }
  else {
    dims[0]=0;
    dims[1]=0;
    n_dims=1;
  }
  return n_dims;
}

typedef struct {
  YSliceOperation sop;
  double *input;
  YMatrixSize size;
  double *output;
  unsigned int output_len;
} SliceOpData;

static
gpointer vector_slice_op_create_data(YOperation *op, gpointer data, YData *input)
{
  if(input==NULL) return NULL;
  SliceOpData *d;
  gboolean neu = TRUE;
  if(data==NULL) {
    d = g_new0(SliceOpData,1);
  } else {
    neu = FALSE;
    d=(SliceOpData *) data;
  }
  YSliceOperation *sop = Y_SLICE_OPERATION(op);
  d->sop = *sop;
  YMatrix *mat = Y_MATRIX(input);
  unsigned int old_nrow = d->size.rows;
  unsigned int old_ncol = d->size.columns;
  d->size = y_matrix_get_size(mat);
  if(!neu) {
    if(old_nrow != d->size.rows || old_ncol != d->size.columns) {
      g_free(d->input);
      d->input = g_new(double,d->size.rows*d->size.columns);
    }
  }
  else {
    d->input = g_new(double,d->size.rows*d->size.columns);
  }
  memcpy(d->input,y_matrix_get_values(mat),d->size.rows*d->size.columns*sizeof(double));
  unsigned int dims[2];
  vector_slice_size(op,input,dims);
  if(d->output_len != dims[0]*dims[1]) {
    g_free(d->output);
    d->output = g_new(double,dims[0]*dims[1]);
  }
  return d;
}

static
void vector_slice_op_data_free(gpointer d)
{
  SliceOpData *s = (SliceOpData *) d;
  g_message("free");
  g_free(s->input);
  g_free(s->output);
  g_free(d);
}

static
gpointer vector_slice_op(gpointer input)
{
  SliceOpData *d = (SliceOpData *) input;

  if(d==NULL)
    return NULL;

  //g_message("task data: index %d, width %d, type %u, input %p, nrow %u, ncol %u",d->index,d->width,d->type,d->input,d->nrow,d->ncol);

  unsigned int nrow = d->size.rows;
  unsigned int ncol = d->size.columns;
  double *m = d->input;

  double *v = d->output;

  if(d->sop.type == SLICE_ROW) {
    memcpy(v,&m[d->sop.index*ncol],sizeof(double)*ncol);
  }
  else if(d->sop.type == SLICE_COL) {
    unsigned int j;
    for(j=0;j<nrow;j++) {
      v[j]=m[d->sop.index+j*ncol];
    }
  }
  else if(d->sop.type == SLICE_SUMROWS) {
    int w = d->sop.width;
    int start=d->sop.index-w/2;
    start=MAX(start,0);
    int end = d->sop.index+w/2;
    end=MIN(end,(int)(nrow-1));
    unsigned int j;
    int k;
    int n=0;
    for(j=0;j<ncol;j++) {
      v[j]=0.;
      for(k=start;k<=end;k++) {
	      v[j]+=m[j+k*ncol];
	      n++;
	    }
	    if(d->sop.mean)
	      v[j]/=n;
    }
  }
  else if(d->sop.type == SLICE_SUMCOLS) {
    int w = d->sop.width;
    int start=d->sop.index-w/2;
    start=MAX(start,0);
    int end = d->sop.index+w/2;
    end=MIN(end,(int)(ncol-1));
    unsigned int j;
    int k;
    int n=0;
    for(j=0;j<nrow;j++) {
      v[j]=0.;
      for(k=start;k<=end;k++) {
        v[j]+=m[k+j*ncol];
        n++;
      }
      if(d->sop.mean)
	      v[j]/=n;
    }
  }
  return v;
}

static void
y_slice_operation_class_init (YSliceOperationClass *slice_klass)
{
  GObjectClass *gobject_klass = (GObjectClass *) slice_klass;
  gobject_klass->set_property = y_slice_operation_set_property;
  gobject_klass->get_property = y_slice_operation_get_property;
  YOperationClass *op_klass = (YOperationClass *) slice_klass;
  op_klass->thread_safe = TRUE;
  op_klass->op_size = vector_slice_size;
  op_klass->op_func = vector_slice_op;
  op_klass->op_data = vector_slice_op_create_data;
  op_klass->op_data_free = vector_slice_op_data_free;

  g_object_class_install_property (gobject_klass, SLICE_PROP_INDEX,
		g_param_spec_int ("index", "Index",
			"Index of slice",
			0, 2000000000, 0,
			G_PARAM_READWRITE));

  g_object_class_install_property (gobject_klass, SLICE_PROP_TYPE,
		g_param_spec_int ("type", "Type",
			"Type of slicing operation",
			SLICE_ROW, 7, SLICE_ROW,
			G_PARAM_READWRITE));

  g_object_class_install_property (gobject_klass, SLICE_PROP_WIDTH,
		g_param_spec_int ("width", "Width",
			"Width of slice, if appropriate",
			1, 2000000000, 1,
			G_PARAM_READWRITE));

	g_object_class_install_property (gobject_klass,
		SLICE_PROP_MEAN,
		g_param_spec_boolean ("mean", "average over elements",
			"Whether to average over elements.",
			FALSE, G_PARAM_READWRITE));
}

static void
y_slice_operation_init(YSliceOperation *slice) {
  g_assert(Y_IS_SLICE_OPERATION(slice));
  slice->index = 0;
  slice->type = SLICE_ROW;
  slice->width = 1;
}

YOperation *y_slice_operation_new (int type, int index, int width)
{
  g_assert(index>=0);
  g_assert(width>0);

  YOperation *o = g_object_new(Y_TYPE_SLICE_OPERATION,"type",type,"index",index,"width",width,NULL);

  return o;
}

void y_slice_operation_set_pars(YSliceOperation *d, int type, int index,
                                 int width)
{
  g_assert(Y_IS_SLICE_OPERATION(d));
  g_assert(index>=0);
  g_assert(width>0);
  if(d->type != type)
    g_object_set(d,"type",type,NULL);
  if(d->index != index)
    g_object_set(d,"index",index,NULL);
  if(d->width != width)
    g_object_set(d,"width",width,NULL);
}
