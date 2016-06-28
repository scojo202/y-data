/*
 * y-data-tcp.h :
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

#ifndef Y_DATA_TCP_H
#define Y_DATA_TCP_H

#include <y-data.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(YDataTcpSender,y_data_tcp_sender,Y,DATA_TCP_SENDER,GObject)

#define Y_TYPE_DATA_TCP_SENDER	(y_data_tcp_sender_get_type ())

void y_data_tcp_server_init (guint16 port);

YDataTcpSender	*y_data_tcp_sender_new      (YData *data, int id);

G_DECLARE_FINAL_TYPE(YScalarTcpReceiver,y_scalar_tcp_receiver,Y,SCALAR_TCP_RECEIVER,YScalar)

#define Y_TYPE_SCALAR_TCP_RECEIVER	(y_scalar_tcp_receiver_get_type ())

YScalarTcpReceiver	*y_scalar_tcp_receiver_new      (const gchar *url, guint16 port, int id);

G_DECLARE_FINAL_TYPE(YVectorTcpReceiver,y_vector_tcp_receiver,Y,VECTOR_TCP_RECEIVER,YVector)

#define Y_TYPE_VECTOR_TCP_RECEIVER	(y_vector_tcp_receiver_get_type ())

YVectorTcpReceiver	*y_vector_tcp_receiver_new      (const gchar *url, guint16 port, int id);

G_END_DECLS

#endif

