/*
 * y-hdf.h :
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

#ifndef Y_HDF_H
#define Y_HDF_H

#include <hdf5.h>
#include <hdf5_hl.h>
#include <y-data-class.h>
#include <y-data-simple.h>

G_BEGIN_DECLS

hid_t y_open_hdf5_file_for_writing(const gchar *filename, gboolean overwrite, GError **err);
hid_t y_open_hdf5_file_for_reading(const gchar *filename, GError **err);

void y_data_attach_h5(YData *d, hid_t group_id, const gchar *data_name);
//YData *y_data_from_h5(hid_t group_id, const gchar *data_name);

void y_vector_attach_h5 (YVector *v, hid_t group_id, const gchar *data_name);
void y_matrix_attach_h5 (YMatrix *m, hid_t group_id, const gchar *data_name);

YData *y_vector_from_h5 (hid_t group_id, const gchar *data_name);
void y_vector_val_replace_h5 (YVectorVal *v, hid_t group_id, const gchar *data_name);

G_END_DECLS

#endif
