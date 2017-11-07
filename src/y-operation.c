/*
 * y-operation.c :
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

#include <string.h>
#include <y-operation.h>

/**
 * SECTION: y-operation
 * @short_description: Object representing operations
 *
 * YOperations are objects that take data and create other data automatically.
 *
 * 
 */

G_DEFINE_ABSTRACT_TYPE(YOperation, y_operation, G_TYPE_OBJECT);

static void y_operation_init(YOperation * op)
{
}

static void y_operation_class_init(YOperationClass * klass)
{
}

double *y_create_input_array_from_vector(YVector * input, gboolean is_new,
					 unsigned int old_size,
					 double *old_input)
{
	double *d = old_input;
	unsigned int size = y_vector_get_len(input);
	if (!is_new) {
		if (old_size != size) {
			g_free(old_input);
			d = g_new(double, size);
		}
	} else {
		d = g_new(double, size);
	}
	memcpy(d, y_vector_get_values(input), size * sizeof(double));
	return d;
}

double *y_create_input_array_from_matrix(YMatrix * input, gboolean is_new,
					 YMatrixSize old_size,
					 double *old_input)
{
	double *d = old_input;
	unsigned int old_nrow = old_size.rows;
	unsigned int old_ncol = old_size.columns;
	YMatrixSize size = y_matrix_get_size(input);
	if (!is_new) {
		if (old_nrow != size.rows || old_ncol != size.columns) {
			g_free(old_input);
			d = g_new(double, size.rows * size.columns);
		}
	} else {
		d = g_new(double, size.rows * size.columns);
	}
	memcpy(d, y_matrix_get_values(input),
	       size.rows * size.columns * sizeof(double));
	return d;
}

/**
 * y_operation_get_task :
 * @op: a #YOperation
 * @user_data: task data
 * @cb: callback that will be called when task is complete
 * @cb_data: data for callback
 *
 * Get the #GTask for an operation.
 *
 **/
GTask *y_operation_get_task(YOperation * op, gpointer user_data,
			    GAsyncReadyCallback cb, gpointer cb_data)
{
	GTask *task = g_task_new(op, NULL, cb, cb_data);

	g_task_set_task_data(task, user_data, NULL);
	return task;
}

static void
task_thread_func(GTask * task,
		 gpointer source_object,
		 gpointer task_data, GCancellable * cancellable)
{
	YOperation *op = (YOperation *) source_object;
	YOperationClass *klass = Y_OPERATION_GET_CLASS(op);
	gpointer output = klass->op_func(task_data);
	g_task_return_pointer(task, output, NULL);
}

/**
 * y_operation_run_task :
 * @op: a #YOperation
 * @user_data: task data
 * @cb: callback that will be called when task is complete
 * @cb_data: data for callback
 *
 * Get the #GTask for an operation and run it in a thread.
 *
 **/
void y_operation_run_task(YOperation * op, gpointer user_data,
			  GAsyncReadyCallback cb, gpointer cb_data)
{
	GTask *task = y_operation_get_task(op, user_data, cb, cb_data);
	g_task_run_in_thread(task, task_thread_func);
	g_object_unref(task);
}

/**
 * y_operation_create_task_data:
 * @op: a #YOperation
 * @input: a #YData to serve as the input
 *
 * Create a task data structure to be used to perform the operation for a given
 * input object. It will make a copy of the data in the input object, so if that
 * changes, the structure must be updated.
 **/
gpointer y_operation_create_task_data(YOperation * op, YData * input)
{
	YOperationClass *klass = Y_OPERATION_GET_CLASS(op);
	return klass->op_data(op, NULL, input);
}

/**
 * y_operation_update_task_data:
 * @op: a #YOperation
 * @task_data: a pointer to the task data
 * @input: a #YData to serve as the input
 *
 * Update an existing task data structure, possibly for a new input object.
 **/
void y_operation_update_task_data(YOperation * op, gpointer task_data,
				  YData * input)
{
	YOperationClass *klass = Y_OPERATION_GET_CLASS(op);
	klass->op_data(op, task_data, input);
}
