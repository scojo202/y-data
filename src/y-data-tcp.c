/*
 * y-data-tcp.c :
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
#include <y-data-tcp.h>

static GSocketService *listener = NULL;
static GSocketConnection *socket_conn = NULL;

static gboolean
conn_incoming (GSocketService *service, GSocketConnection *connection,
	       GObject *source_object, gpointer user_data)
{
  g_object_ref(connection);
  socket_conn = connection;
  g_info("Incoming connection received\n");
  return FALSE;
}

void y_data_tcp_server_init (guint16 port)
{
  if(listener==NULL)
    listener = g_socket_service_new();
  gboolean retval = g_socket_listener_add_inet_port(G_SOCKET_LISTENER(listener),port,NULL,NULL);
  g_assert(retval);
  g_signal_connect(G_OBJECT(listener), "incoming",
		   G_CALLBACK(conn_incoming), NULL);
}

struct _YDataTcpSender {
	GObject      base;
	YData        *data;
        int id;
};

G_DEFINE_TYPE (YDataTcpSender, y_data_tcp_sender, G_TYPE_OBJECT);

static void
y_data_tcp_sender_class_init (YDataTcpSenderClass *klass)
{
	GObjectClass *gobject_klass = (GObjectClass *) klass;
}

static void
y_data_tcp_sender_init(YDataTcpSender *val)
{}

static void
on_data_changed_after(YData *data, gpointer   user_data) {
  /* send it over the socket */
  GOutputStream *s = g_io_stream_get_output_stream(G_IO_STREAM(socket_conn));
  /* send id */
  /* send type number */
  /* send length */
  /* send data */
}

YDataTcpSender	*y_data_tcp_sender_new      (YData *data, int id)
{
        YDataTcpSender *res = g_object_new (Y_TYPE_DATA_TCP_SENDER, NULL);
        res->data = data;
        res->id = id;
        /* connect to changed signal */
        g_signal_connect_after(res->data,"changed",G_CALLBACK(on_data_changed_after),res);
        return res;
}

struct _YScalarTcpReceiver {
	YScalar      base;
        gchar *url;
	guint16 port;
        int id;
};

G_DEFINE_TYPE (YScalarTcpReceiver, y_scalar_tcp_receiver, Y_TYPE_SCALAR);

static void
y_scalar_tcp_receiver_class_init (YScalarTcpReceiverClass *klass)
{
	GObjectClass *gobject_klass = (GObjectClass *) klass;
}

static void
y_scalar_tcp_receiver_init(YScalarTcpReceiver *val)
{}

YScalarTcpReceiver	*y_scalar_tcp_receiver_new      (const gchar *url, guint16 port, int id)
{
  return NULL;
}

struct _YVectorTcpReceiver {
	YVector      base;
        gchar *url;
	guint16 port;
        int id;
};

G_DEFINE_TYPE (YVectorTcpReceiver, y_vector_tcp_receiver, Y_TYPE_VECTOR);

static void
y_vector_tcp_receiver_class_init (YVectorTcpReceiverClass *klass)
{
	GObjectClass *gobject_klass = (GObjectClass *) klass;
}

static void
y_vector_tcp_receiver_init(YVectorTcpReceiver *val)
{}

YVectorTcpReceiver	*y_vector_tcp_receiver_new      (const gchar *url, guint16 port, int id)
{
   return NULL;
}

