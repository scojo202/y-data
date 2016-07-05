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
        gint id;
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
  YDataTcpSender *sender = (YDataTcpSender *) user_data;
  /* send it over the socket */
  GOutputStream *s;
  if(socket_conn)
    s = g_io_stream_get_output_stream(G_IO_STREAM(socket_conn));
  else return;
  /* send id */
  /* send type */
  /* send dimensions */
  gint header[3];
  header[0]=g_htonl(sender->id);
  int n_dims = y_data_get_n_dimensions (data);
  if(n_dims==2) {
    YMatrixSize s = y_matrix_get_size (Y_MATRIX(data));
    header[1]=g_htonl(s.columns);
    header[2]=g_htonl(s.rows);
  }
  else if (n_dims==1) {
    header[1]=g_htonl(y_vector_get_len(Y_VECTOR(data)));
    header[2]=0;
  }
  else if (n_dims==0){
    header[1]=0;
    header[2]=0;
  }
  g_output_stream_write(s,header,3*sizeof(gint),NULL,NULL);
  /* send data */
  if(n_dims==0) {
    double d = y_scalar_get_value(Y_SCALAR(data));
    guint64 i = GUINT64_TO_LE(*(guint64 *)(&d));
    g_output_stream_write(s,&i,sizeof(guint64),NULL,NULL);
  }
  
}

YDataTcpSender	*y_data_tcp_sender_new      (YData *data, gint id)
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
	GSocketConnection *socket_conn;
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

static
gboolean poll_func(GObject *pollable_stream,gpointer user_data)
{
  YScalarTcpReceiver *r = (YScalarTcpReceiver *) user_data;
  gint buffer[3];
  g_pollable_input_stream_read_nonblocking(pollable_stream,buffer,3*sizeof(gint),NULL,NULL);
  if(g_ntohl(buffer[0])==r->id) {
    //g_message("hello (%d %d)",g_ntohl(buffer[1]),g_ntohl(buffer[2]));
    g_assert(buffer[1]==0 && buffer[2]==0);
    guint64 i;
    g_pollable_input_stream_read_nonblocking(pollable_stream,&i,sizeof(guint64),NULL,NULL);
    guint64 i2 = GUINT64_FROM_LE(i);
    double d = *(double *)&i2;
    g_message("read %e",d);
  }
  
  return TRUE;
}

YScalarTcpReceiver	*y_scalar_tcp_receiver_new      (const gchar *url, guint16 port, guint16 id)
{
  YScalarTcpReceiver *r = g_object_new(Y_TYPE_SCALAR_TCP_RECEIVER,NULL);
  r->id = id;
  GSocketClient *client = g_socket_client_new();
  r->socket_conn = g_socket_client_connect_to_host(client,url,port,NULL,NULL);
  GInputStream *i = g_io_stream_get_input_stream(r->socket_conn);
  GSource *source = g_pollable_input_stream_create_source (G_POLLABLE_INPUT_STREAM(i), NULL);
  g_source_set_callback(source,poll_func,r,NULL);
  g_source_attach(source,NULL);
  return r;
}

struct _YVectorTcpReceiver {
	YVector      base;
        gchar *url;
	guint16 port;
        guint16 id;
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

YVectorTcpReceiver	*y_vector_tcp_receiver_new      (const gchar *url, guint16 port, guint16 id)
{
   return NULL;
}

