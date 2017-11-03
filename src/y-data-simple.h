/*
 * y-data-simple.h :
 *
 * Copyright (C) 2003-2004 Jody Goldberg (jody@gnome.org)
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
 
#ifndef Y_DATA_SIMPLE_H
#define Y_DATA_SIMPLE_H

#include <glib-object.h>
#include <y-data-class.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(YScalarVal,y_scalar_val,Y,SCALAR_VAL,YScalar)

#define Y_TYPE_SCALAR_VAL	(y_scalar_val_get_type ())

YData	*y_scalar_val_new      (double val);
double *y_scalar_val_get_val (YScalarVal *s);

G_DECLARE_FINAL_TYPE(YVectorVal,y_vector_val,Y,VECTOR_VAL,YVector)

#define Y_TYPE_VECTOR_VAL  (y_vector_val_get_type ())

YData	*y_vector_val_new      (double *val, unsigned n, GDestroyNotify   notify);
YData	*y_vector_val_new_alloc (unsigned n);
YData	*y_vector_val_new_copy (const double *val, unsigned n);

double *y_vector_val_get_array (YVectorVal *s);
void y_vector_val_replace_array(YVectorVal *s, double *array, unsigned n, GDestroyNotify notify);

G_DECLARE_FINAL_TYPE(YMatrixVal,y_matrix_val,Y,MATRIX_VAL,YMatrix)

#define Y_TYPE_MATRIX_VAL  (y_matrix_val_get_type ())

YData	*y_matrix_val_new      (double *val, unsigned rows, unsigned columns, GDestroyNotify   notify);
YData *y_matrix_val_new_copy (const double   *val,
                                     unsigned  rows, unsigned columns);
YData *y_matrix_val_new_alloc (unsigned n, unsigned m);
                                     
double *y_matrix_val_get_array (YMatrixVal *s);
void y_matrix_val_replace_array(YMatrixVal *s, double *array, unsigned rows, unsigned columns, GDestroyNotify notify);

G_DECLARE_FINAL_TYPE(YThreeDArrayVal,y_three_d_array_val,Y,THREE_D_ARRAY_VAL,YThreeDArray)

#define Y_TYPE_THREE_D_ARRAY_VAL  (y_three_d_array_val_get_type ())

YData	*y_three_d_array_val_new      (double *val, unsigned rows, unsigned columns, unsigned layers, GDestroyNotify   notify);
YData *y_three_d_array_val_new_copy (double   *val,
                                     unsigned  rows, unsigned columns, unsigned layers);
YData *y_three_d_array_val_new_alloc (unsigned n, unsigned m, unsigned l);
                                     
double *y_three_d_array_val_get_array (YThreeDArrayVal *s);
                                     
G_END_DECLS

#endif /* Y_DATA_SIMPLE_H */
