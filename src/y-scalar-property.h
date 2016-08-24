/*
 * y-scalar-property.h :
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

#ifndef Y_SCALAR_PROP_H
#define Y_SCALAR_PROP_H

#include <y-data.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(YScalarProperty,y_scalar_property,Y,SCALAR_PROPERTY,YScalar)

#define Y_TYPE_SCALAR_PROPERTY	(y_scalar_property_get_type ())

YScalarProperty	*y_scalar_property_new      (GObject *obj, const gchar *name);

G_END_DECLS

#endif

