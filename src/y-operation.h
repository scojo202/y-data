/*
 * y-operation.h :
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

/*  Object for an operation  */
/*  Operations maintain copies of input data for multithreading */

#ifndef Y_OP_H
#define Y_OP_H

#include <gio/gio.h>
#include <y-data-class.h>

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(YOperation,y_operation,Y,OPERATION,GObject)

#define Y_TYPE_OPERATION	(y_operation_get_type ())

/**
 * YOperationClass:
 * @base: base class.
 * @thread_safe: whether the operation can be run in a thread.
 *
 * Class for YOperation.
 **/

struct _YOperationClass {
	GObjectClass base;
	gboolean thread_safe; /* does this operation keep copies of all data so it can be done in a thread? */

	int (*op_size) (YOperation *op, YData *input, unsigned int *dims);
  gpointer (*op_func) (gpointer data);
  gpointer (*op_data) (YOperation *op, gpointer data, YData *input);
  GDestroyNotify op_data_free;
};

GTask * y_operation_get_task(YOperation *op, gpointer user_data, GAsyncReadyCallback cb, gpointer cb_data);
gpointer y_operation_create_task_data(YOperation *op, YData *input);
void y_operation_run_task(YOperation *op, gpointer user_data, GAsyncReadyCallback cb, gpointer cb_data);
void y_operation_update_task_data(YOperation *op, gpointer task_data, YData *input);

G_END_DECLS

#endif

