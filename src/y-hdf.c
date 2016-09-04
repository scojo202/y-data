/*
 * y-hdf.c :
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

#include <gio/gio.h>
#include <y-hdf.h>

#define DEFLATE_LEVEL 5

/**
 * SECTION: y-hdf
 * @short_description: Functions for saving and loading from HDF5 files
 *
 **/

hid_t y_open_hdf5_file_for_writing(const gchar *filename, GError **err) {
  /* make sure file doesn't already exist */
  GFile *file = g_file_new_for_path(filename);
  gboolean exists = g_file_query_exists(file,NULL);
  g_object_unref(file);
  if(exists) {
    g_set_error(err,G_IO_ERROR,G_IO_ERROR_EXISTS,"File not found: %s",filename);
    return 0;
  }
  hid_t hfile = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  return hfile;
}

hid_t y_open_hdf5_file_for_reading(const gchar *filename, GError **err) {
  /* make sure file exists */
  GFile *file = g_file_new_for_path(filename);
  gboolean exists = g_file_query_exists(file,NULL);
  g_object_unref(file);
  if(!exists) {
    g_set_error(err,G_IO_ERROR,G_IO_ERROR_NOT_FOUND,"File not found: %s",filename);
    return 0;
  }
  hid_t hfile = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
  return hfile;
}

/**
 * y_vector_attach_h5: (skip)
 * @v: #YVector
 * @group_id: HDF5 group
 * @data_name: name
 *
 * Add a vector to an HDF5 group.
 **/
 
void y_vector_attach_h5 (YVector *v, hid_t group_id, const gchar *data_name)
{
  g_return_if_fail(Y_IS_VECTOR(v));
  g_return_if_fail(group_id != 0);
  hsize_t dims[1] = {y_vector_get_len(v)};

  hid_t dataspace_id = H5Screate_simple(1, dims, NULL);
  hid_t plist_id = H5Pcreate(H5P_DATASET_CREATE);

  H5Pset_chunk(plist_id, 1, dims);
  H5Pset_deflate(plist_id, DEFLATE_LEVEL);

  hid_t id = H5Dcreate2(group_id, data_name, H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
  const double *data = y_vector_get_values(v);
  H5Dwrite(id, H5T_NATIVE_DOUBLE, dataspace_id, dataspace_id, H5P_DEFAULT, data);

  H5Sclose(dataspace_id);

  H5Pclose(plist_id);
  H5Dclose(id);
}

/**
 * y_matrix_attach_h5: (skip)
 * @m: #YMatrix
 * @group_id: HDF5 group
 * @data_name: name
 *
 * Add a matrix to an HDF5 group.
 **/
void y_matrix_attach_h5 (YMatrix *m, hid_t group_id, const gchar *data_name)
{
  g_return_if_fail(Y_IS_MATRIX(m));
  g_return_if_fail(group_id != 0);
  hsize_t dims[2] = {y_matrix_get_rows(m),y_matrix_get_columns(m)};

  hid_t dataspace_id = H5Screate_simple(2, dims, NULL);
  hid_t plist_id = H5Pcreate(H5P_DATASET_CREATE);

  H5Pset_chunk(plist_id, 2, dims);
  H5Pset_deflate(plist_id, DEFLATE_LEVEL);

  hid_t id = H5Dcreate2(group_id, data_name, H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
  H5Dwrite(id, H5T_NATIVE_DOUBLE, dataspace_id, dataspace_id, H5P_DEFAULT, y_matrix_get_values(m));

  H5Sclose(dataspace_id);

  H5Pclose(plist_id);
  H5Dclose(id);
}

static
void save_func(gpointer key, gpointer value, gpointer user_data)
{
  const gchar *name = (gchar *) key;
  g_message("save %s",name);
  YData *d = Y_DATA(value);
  hid_t *subgroup_id = (hid_t *) user_data;
  y_data_attach_h5(d,*subgroup_id,name);
}

/**
 * y_data_attach_h5: (skip)
 * @d: #YData
 * @group_id: HDF5 group
 * @data_name: name
 *
 * Add a YData object to an HDF5 group.
 **/
void y_data_attach_h5 (YData *d, hid_t group_id, const gchar *data_name)
{
  hid_t subgroup_id;
  g_return_if_fail(Y_IS_DATA(d));
  g_return_if_fail(group_id != 0);
  char n = y_data_get_n_dimensions(d);
  switch(n) {
  case -1:
    if(data_name)
      subgroup_id = H5Gcreate(group_id,data_name,H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    else
      subgroup_id = group_id;
    y_struct_foreach(Y_STRUCT(d),save_func,&subgroup_id);
    if(data_name != NULL)
      H5Gclose(subgroup_id);
    break;
  case 0:
    g_warning("scalar save to h5 not implemented");
    break;
  case 1:
    y_vector_attach_h5(Y_VECTOR(d),group_id,data_name);
    break;
  case 2:
    y_matrix_attach_h5(Y_MATRIX(d),group_id,data_name);
    break;
  default:
    g_warning("number of dimensions %d not supported",n);
    return;
  }
}

/**
 * y_vector_from_h5: (skip)
 * @group_id: HDF5 group
 * @data_name: name
 *
 * Read a vector from an HDF5 group.
 *
 * Returns: (transfer full): The vector.
 **/
YData *y_vector_from_h5 (hid_t group_id, const gchar *data_name)
{
  g_return_val_if_fail(group_id != 0,NULL);
  hid_t dataset_h5 = H5Dopen(group_id,data_name,H5P_DEFAULT);
  hid_t dspace_id = H5Dget_space(dataset_h5);
  int rank;
  hsize_t  *current_dims;
  hsize_t  *max_dims;
  rank=H5Sget_simple_extent_ndims(dspace_id);
  current_dims= (hsize_t *)g_malloc(rank*sizeof(hsize_t));
  max_dims=(hsize_t *)g_malloc(rank*sizeof(hsize_t));
  H5Sget_simple_extent_dims(dspace_id, current_dims, max_dims);
  g_assert(rank==1);
  g_assert(current_dims[0]>0);
  double *d = g_new(double,current_dims[0]);
  H5Dread(dataset_h5,H5T_NATIVE_DOUBLE, H5S_ALL, dspace_id, H5P_DEFAULT, d);
  YData *y = y_vector_val_new(d,current_dims[0],g_free);
  return y;
}

/**
 * y_vector_val_replace_h5: (skip)
 * @vec: YVectorVal
 * @group_id: HDF5 group
 * @data_name: name
 *
 * Load contents of HDF5 dataset into a vector, replacing its contents.
 **/
void y_vector_val_replace_h5 (YVectorVal *v, hid_t group_id, const gchar *data_name)
{
  g_return_if_fail(group_id != 0);
  hid_t dataset_h5 = H5Dopen(group_id,data_name,H5P_DEFAULT);
  hid_t dspace_id = H5Dget_space(dataset_h5);
  int rank;
  hsize_t  *current_dims;
  hsize_t  *max_dims;
  rank=H5Sget_simple_extent_ndims(dspace_id);
  current_dims= (hsize_t *)g_malloc(rank*sizeof(hsize_t));
  max_dims=(hsize_t *)g_malloc(rank*sizeof(hsize_t));
  H5Sget_simple_extent_dims(dspace_id, current_dims, max_dims);
  g_assert(rank==1);
  g_assert(current_dims[0]>0);
  double *d = g_new(double,current_dims[0]);
  H5Dread(dataset_h5,H5T_NATIVE_DOUBLE, H5S_ALL, dspace_id, H5P_DEFAULT, d);
  y_vector_val_replace_array(v,d,current_dims[0]);
}
