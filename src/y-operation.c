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

#include <y-operation.h>

/**
 * SECTION: y-operation
 * @short_description: Object representing an operation that takes data and creates other data.
 *
 * 
 *
 * 
 */

G_DEFINE_ABSTRACT_TYPE (YOperation, y_operation, G_TYPE_OBJECT);

static void
y_operation_init (YOperation *op)
{
}

static void
y_operation_class_init (YOperationClass *klass)
{
}

GTask * y_operation_get_task(YOperation *op,gpointer user_data, GAsyncReadyCallback cb, gpointer cb_data)
{
  GTask *task = g_task_new(op,NULL,cb,cb_data);
  YOperationClass *klass = Y_OPERATION_GET_CLASS (op);

  g_task_set_task_data(task,user_data,NULL);
  return task;
}

static void
task_thread_func(GTask        *task,
                 gpointer      source_object,
                 gpointer      task_data,
                 GCancellable *cancellable)
{
  YOperation *op = (YOperation *) source_object;
  YOperationClass *klass = Y_OPERATION_GET_CLASS (op);
  gpointer output = klass->op_func(task_data);
  g_task_return_pointer(task,output,NULL);
}

void y_operation_run_task(YOperation *op,gpointer user_data, GAsyncReadyCallback cb, gpointer cb_data)
{
  GTask *task = y_operation_get_task(op,user_data,cb,cb_data);
  g_task_run_in_thread(task,task_thread_func);
  g_object_unref(task);
}

gpointer y_operation_create_task_data(YOperation *op, YData *input)
{
  YOperationClass *klass = Y_OPERATION_GET_CLASS (op);
  return klass->op_data(op,NULL,input);
}

void y_operation_update_task_data(YOperation *op, gpointer task_data, YData *input)
{
  YOperationClass *klass = Y_OPERATION_GET_CLASS (op);
  klass->op_data(op,task_data,input);
}

