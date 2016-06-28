#include <math.h>
#include <y-data-simple.h>
#include <y-hdf.h>
#include <y-data-vector-slice.h>
#include <y-data-derived.h>
#include <hdf5.h>

YData *d1, *d2, *d3;

#define DATA_COUNT 20000

#define A 1e-19

static void
build_scalar_val (void)
{
  gint i;
  double t;
  double x, y, z;

  x = 1.4;
  y = 3.4;
  z = 3.2;
  
  d1 = y_scalar_val_new (x);
  d2 = y_scalar_val_new (y);
  d3 = y_scalar_val_new (z);
}

static void
build_vector_val (void)
{
  gint i;
  double t;
  double *x, *y, *z;

  x=g_malloc(sizeof(double)*DATA_COUNT);
  y=g_malloc(sizeof(double)*DATA_COUNT);
  z=g_malloc(sizeof(double)*DATA_COUNT);
  for (i=0; i<DATA_COUNT; ++i) {
    t = 2*G_PI*i/(double)DATA_COUNT;
    x[i] = 2*sin (4*t)*A;
    y[i] = cos (3*t);
    z[i] = cos (5*t)*A;
  }
  d1 = y_vector_val_new (x, DATA_COUNT, g_free);
  d2 = y_vector_val_new (y, DATA_COUNT, g_free);
  d3 = y_vector_val_new_copy (z, DATA_COUNT);
  g_free(z);
}

static void
build_matrix_val (void)
{
  gint i,j;
  double t;
  double *x, *y, *z;

  x=g_malloc(sizeof(double)*DATA_COUNT*100);
  y=g_malloc(sizeof(double)*DATA_COUNT*100);
  z=g_malloc(sizeof(double)*DATA_COUNT*100);
  for (i=0; i<DATA_COUNT; ++i) {
    for (j=0; j<100; ++j) {
      t = 2*G_PI*(i+j)/(double)DATA_COUNT;
      x[i+j*DATA_COUNT] = 2*sin (4*t)*A;
      y[i+j*DATA_COUNT] = cos (3*t);
      z[i+j*DATA_COUNT] = cos (5*t)*A;
    }
  }
  d1 = y_matrix_val_new (x, DATA_COUNT,100, g_free);
  d2 = y_matrix_val_new (y, DATA_COUNT,100, g_free);
  d3 = y_matrix_val_new_copy (z, DATA_COUNT,100);
  g_free(z);
}

int
main (int argc, char *argv[])
{
  YStruct *s = g_object_new(Y_TYPE_STRUCT,NULL);
  
  build_scalar_val ();
  
  y_struct_set_data(s,"scalar_data1",d1);
  y_struct_set_data(s,"scalar_data2",d2);
  g_object_unref(d3);
  
  build_vector_val ();
  
  y_struct_set_data(s,"vector_data1",d1);
  y_struct_set_data(s,"vector_data2",d2);
  g_object_unref(d3);
  
  build_matrix_val ();
  
  y_struct_set_data(s,"matrix_data1",d1);
  y_struct_set_data(s,"matrix_data2",d2);
  g_object_unref(d3);

  /* test slice */
  YSliceOperation *op1 = y_slice_operation_new(SLICE_ROWS,50,10);
  YSliceOperation *op2 = y_slice_operation_new(SLICE_COLS,20,10);
  YVectorDerived *der1 = y_vector_derived_new(d1,op1);
  YVectorDerived *der2 = y_vector_derived_new(d2,op1);
  YVectorDerived *der3 = y_vector_derived_new(d2,op2);
  
  y_struct_set_data(s,"slice1",der1);
  y_struct_set_data(s,"slice2",der2);
  y_struct_set_data(s,"slice3",der3);
  
  y_vector_derived_set_autorun(der3,TRUE);
  y_data_emit_changed(d2);

  g_usleep(2000000);

  hid_t hfile = H5Fcreate("test.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  y_data_attach_h5(s,hfile,NULL);
  H5Fclose(hfile);
  
  g_object_unref(s);

  return 0;
}
