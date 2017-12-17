/*
 * y-vector-ring.h :
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
 
#ifndef Y_VECTOR_RING_H
#define Y_VECTOR_RING_H

#include <glib-object.h>
#include <y-data-class.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(YVectorRing,y_vector_ring,Y,VECTOR_RING,YVector)

#define Y_TYPE_VECTOR_RING  (y_vector_ring_get_type ())

YData *y_vector_ring_new (unsigned nmax, unsigned n, gboolean track_timestamps);
void y_vector_ring_set_length(YVectorRing *d, unsigned newlength);
void y_vector_ring_append(YVectorRing *d, double val);
void y_vector_ring_append_array(YVectorRing *d, double *arr, int len);

void y_vector_ring_set_source(YVectorRing *d, YScalar *source);

YVectorRing *y_vector_ring_get_timestamps(YVectorRing *d);

G_DECLARE_FINAL_TYPE(YRingMatrix,y_ring_matrix,Y,RING_MATRIX,YMatrix)

#define Y_TYPE_RING_MATRIX  (y_ring_matrix_get_type ())

YData *y_ring_matrix_new (unsigned c, unsigned rmax, unsigned r, gboolean track_timestamps);
void y_ring_matrix_set_rows(YRingMatrix *d, unsigned r);
void y_ring_matrix_set_max_rows(YRingMatrix *d, unsigned rmax);
void y_ring_matrix_append(YRingMatrix *d, const double *values, unsigned len);
void y_ring_matrix_set_source(YRingMatrix *d, YVector *source);

YVectorRing *y_ring_matrix_get_timestamps(YRingMatrix *d);

G_END_DECLS

#endif
