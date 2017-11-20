/*
 * y-linear-range.h :
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

#ifndef Y_LINEAR_RANGE_H
#define Y_LINEAR_RANGE_H

#include <y-data-class.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(YLinearRangeVector,y_linear_range_vector,Y,LINEAR_RANGE_VECTOR,YVector)

#define Y_TYPE_LINEAR_RANGE_VECTOR  (y_linear_range_vector_get_type ())

YData	*y_linear_range_vector_new  (double v0, double dv, unsigned n);

void y_linear_range_vector_set_length(YLinearRangeVector *d, unsigned newlength);
void y_linear_range_vector_set_pars(YLinearRangeVector *d, double v0, double dv);

G_DECLARE_FINAL_TYPE(YFourierLinearRangeVector,y_fourier_linear_range_vector,Y,FOURIER_LINEAR_RANGE_VECTOR,YVector)

#define Y_TYPE_FOURIER_LINEAR_RANGE_VECTOR  (y_fourier_linear_range_vector_get_type ())

YData *y_fourier_linear_range_vector_new( YLinearRangeVector *v);

G_END_DECLS

#endif

