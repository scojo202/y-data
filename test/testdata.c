#include <math.h>
#include <y-data.h>

static void
test_simple_scalar(void)
{
  g_autoptr(YValScalar) sv = Y_VAL_SCALAR(y_val_scalar_new (1.0));
  g_assert_cmpfloat (1.0, ==, y_scalar_get_value (Y_SCALAR(sv)));
  g_assert_cmpint (0, ==, y_data_get_n_dimensions (Y_DATA(sv)));
  g_assert_cmpint (1, ==, y_data_get_n_values (Y_DATA(sv)));
  g_assert_true(y_data_has_value(Y_DATA(sv)));

  g_assert_cmpstr("1.0",==, y_scalar_get_str(Y_SCALAR(sv),"%1.1f"));
}

static void
test_simple_vector_new(void)
{
  double *vals = g_malloc(sizeof(double)*100);
  int i;
  for(i=0;i<100;i++) {
    vals[i]=(double) i;
  }
  g_autoptr(YValVector) vv = Y_VAL_VECTOR(y_val_vector_new (vals,100,g_free));
  g_assert_cmpfloat (1.0, ==, y_vector_get_value (Y_VECTOR(vv),1));
  g_assert_cmpint (1, ==, y_data_get_n_dimensions (Y_DATA(vv)));
  g_assert_cmpint (100, ==, y_data_get_n_values (Y_DATA(vv)));
  g_assert_true(y_data_has_value(Y_DATA(vv)));
  double *vals2 = y_val_vector_get_array (vv);
  g_assert_true(vals2==vals);
  g_assert_cmpmem(vals,sizeof(double)*100,vals2,sizeof(double)*100);

  g_assert_cmpint (100, ==, y_vector_get_len (Y_VECTOR(vv)));
  const double *vals3 = y_vector_get_values(Y_VECTOR(vv));
  g_assert_cmpmem(vals,sizeof(double)*100,vals3,sizeof(double)*100);

  for(i=0;i<100;i++) {
    g_assert_cmpfloat(y_vector_get_value(Y_VECTOR(vv),i),==,(double)i);
  }
  g_assert_cmpstr(y_vector_get_str(Y_VECTOR(vv),89,"%1.1f"),==,"89.0");
  g_assert_true(y_vector_is_varying_uniformly(Y_VECTOR(vv)));
  double mn, mx;
  y_vector_get_minmax(Y_VECTOR(vv),&mn,&mx);
  g_assert_cmpfloat(mn, ==, 0.0);
  g_assert_cmpfloat(mx, ==, 99.0);
}

static void
test_simple_vector_alloc(void)
{
  g_autoptr(YValVector) vv = Y_VAL_VECTOR(y_val_vector_new_alloc (100));
  double *vals = y_val_vector_get_array(vv);
  int i;
  for(i=0;i<100;i++) {
    vals[i]=(double) i;
  }
  g_assert_cmpfloat (1.0, ==, y_vector_get_value (Y_VECTOR(vv),1));
  g_assert_cmpint (1, ==, y_data_get_n_dimensions (Y_DATA(vv)));
  g_assert_cmpint (100, ==, y_data_get_n_values (Y_DATA(vv)));
  g_assert_true(y_data_has_value(Y_DATA(vv)));
  const double *vals2 = y_vector_get_values (Y_VECTOR(vv));
  g_assert_true(vals2==vals);
  g_assert_cmpmem(vals,sizeof(double)*100,vals2,sizeof(double)*100);

  for(i=0;i<100;i++) {
    g_assert_cmpfloat(y_vector_get_value(Y_VECTOR(vv),i),==,(double)i);
  }
  g_assert_cmpstr(y_vector_get_str(Y_VECTOR(vv),89,"%1.1f"),==,"89.0");
  g_assert_true(y_vector_is_varying_uniformly(Y_VECTOR(vv)));
  double mn, mx;
  y_vector_get_minmax(Y_VECTOR(vv),&mn,&mx);
  g_assert_cmpfloat(mn, ==, 0.0);
  g_assert_cmpfloat(mx, ==, 99.0);
}

static void
test_simple_vector_copy(void)
{
  double *vals0 = g_malloc(sizeof(double)*100);
  int i;
  for(i=0;i<100;i++) {
    vals0[i]=(double) i;
  }
  g_autoptr(YValVector) vv = Y_VAL_VECTOR(y_val_vector_new_copy (vals0,100));
  double *vals = y_val_vector_get_array(vv);
  for(i=0;i<100;i++) {
    vals[i]=(double) i;
  }
  g_assert_cmpfloat (1.0, ==, y_vector_get_value (Y_VECTOR(vv),1));
  g_assert_cmpint (1, ==, y_data_get_n_dimensions (Y_DATA(vv)));
  g_assert_cmpint (100, ==, y_data_get_n_values (Y_DATA(vv)));
  g_assert_true(y_data_has_value(Y_DATA(vv)));
  double *vals2 = y_val_vector_get_array (vv);
  g_assert_false(vals0==vals);
  g_assert_cmpmem(vals,sizeof(double)*100,vals2,sizeof(double)*100);

  for(i=0;i<100;i++) {
    g_assert_cmpfloat(y_vector_get_value(Y_VECTOR(vv),i),==,(double)i);
  }
  g_assert_cmpstr(y_vector_get_str(Y_VECTOR(vv),89,"%1.1f"),==,"89.0");
  g_assert_true(y_vector_is_varying_uniformly(Y_VECTOR(vv)));
  double mn, mx;
  y_vector_get_minmax(Y_VECTOR(vv),&mn,&mx);
  g_assert_cmpfloat(mn, ==, 0.0);
  g_assert_cmpfloat(mx, ==, 99.0);
}

static void
test_range_vectors(void)
{
  YLinearRangeVector *r = Y_LINEAR_RANGE_VECTOR(y_linear_range_vector_new(0.0,1.0,10));
  g_assert_cmpuint(10, ==, y_vector_get_len(Y_VECTOR(r)));
  g_assert_cmpfloat(0.0, ==, y_linear_range_vector_get_v0(r));
  g_assert_cmpfloat(1.0, ==, y_linear_range_vector_get_dv(r));
  int i;
  for(i=0;i<10;i++) {
    g_assert_cmpfloat(0.0+1.0*i, ==, y_vector_get_value(Y_VECTOR(r),i));
  }
  g_assert_cmpstr(y_vector_get_str(Y_VECTOR(r),8,"%1.1f"),==,"8.0");
  g_assert_true(y_vector_is_varying_uniformly(Y_VECTOR(r)));
  double mn, mx;
  y_vector_get_minmax(Y_VECTOR(r),&mn,&mx);
  g_assert_cmpfloat(mn, ==, 0.0);
  g_assert_cmpfloat(mx, ==, 9.0);

  YFourierLinearRangeVector *f = Y_FOURIER_LINEAR_RANGE_VECTOR(y_fourier_linear_range_vector_new(r));
  g_assert_cmpuint(10/2+1,==,y_vector_get_len(Y_VECTOR(f)));
  g_assert_cmpfloat(0.0, ==, y_vector_get_value(Y_VECTOR(f),0));
  g_assert_true(y_vector_is_varying_uniformly(Y_VECTOR(f)));
  g_object_unref(f);
}

static void
test_ring_vector(void)
{
  YRingVector *r = Y_RING_VECTOR(y_ring_vector_new(100, 0, FALSE));
  g_assert_cmpuint(0, ==, y_vector_get_len(Y_VECTOR(r)));
  int i;
  for(i=0;i<10;i++) {
    y_ring_vector_append(r,(double)i);
  }
  g_assert_cmpuint(10, ==, y_vector_get_len(Y_VECTOR(r)));
  for(i=0;i<10;i++) {
    g_assert_cmpfloat((double)i, ==, y_vector_get_value(Y_VECTOR(r),i));
  }
  g_assert_true(y_vector_is_varying_uniformly(Y_VECTOR(r)));
  y_ring_vector_set_length(r,5);
  g_assert_cmpuint(5, ==, y_vector_get_len(Y_VECTOR(r)));
  g_object_unref(r);
}

static void
test_derived_vector(void)
{
}

int
main (int argc, char *argv[])
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/YData/simple/scalar",test_simple_scalar);
  g_test_add_func("/YData/simple/vector_new",test_simple_vector_new);
  g_test_add_func("/YData/simple/vector_alloc",test_simple_vector_alloc);
  g_test_add_func("/YData/simple/vector_copy",test_simple_vector_copy);
  g_test_add_func("/YData/range/range",test_range_vectors);
  g_test_add_func("/YData/ring/vector",test_ring_vector);
  g_test_add_func("/YData/derived/vector",test_derived_vector);

  return g_test_run();
}
