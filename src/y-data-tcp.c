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
static GList *socket_conns = NULL;

static gboolean
conn_incoming (GSocketService *service, GSocketConnection *connection,
	       GObject *source_object, gpointer user_data)
{
  g_object_ref(connection);
  socket_conns = g_list_prepend(socket_conns,connection);
  //g_info("Incoming connection received\n");
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
  if(socket_conns==NULL) return;
  /* send id, dimensions */
  gint header[3];
  header[0]=g_htonl(sender->id);
  int n_dims = y_data_get_n_dimensions (data);
  g_message("dims: %d",n_dims);
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
  GList *iter;
  for(iter = socket_conns;iter!=NULL;iter=iter->next) {
    s = g_io_stream_get_output_stream(G_IO_STREAM(iter->data));
    //g_message("%d",g_io_stream_is_closed(G_IO_STREAM(iter->data)));
    g_output_stream_write(s,header,3*sizeof(gint),NULL,NULL);
    /* send data */
    if(n_dims==1) {
      const double *d = y_vector_get_values(Y_VECTOR(data));
      int i;
      const int len = y_vector_get_len(Y_VECTOR(data));
      guint64 ii[len];
      for(i=0;i<len;i++) {
        double dd = d[i];
        gpointer pd = &dd;
        ii[i]=GUINT64_TO_BE(*(guint64 *)(pd));
      }
      g_output_stream_write(s,&ii,len*sizeof(guint64),NULL,NULL);
    }
    else if(n_dims==0) {
      double d = y_scalar_get_value(Y_SCALAR(data));
      gpointer pd = &d;
      guint64 i = GUINT64_TO_BE(*(guint64 *)(pd));
      g_output_stream_write(s,&i,sizeof(guint64),NULL,NULL);
    }
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
  double val;
  GSocketConnection *socket_conn;
  gchar *url;
  guint16 port;
  int id;
};

G_DEFINE_TYPE (YScalarTcpReceiver, y_scalar_tcp_receiver, Y_TYPE_SCALAR);

static YData *
y_scalar_tcp_receiver_dup (YData const *src)
{
	YScalarTcpReceiver *dst = g_object_new (G_OBJECT_TYPE (src), NULL);
	YScalarTcpReceiver const *src_val = (YScalarTcpReceiver const *)src;
	dst->val = src_val->val;
	return Y_DATA (dst);
}

static double
y_scalar_tcp_receiver_get_value (YScalar *dat)
{
	YScalarTcpReceiver const *sval = (YScalarTcpReceiver const *)dat;
	return sval->val;
}

static void
y_scalar_tcp_receiver_class_init (YScalarTcpReceiverClass *klass)
{
  GObjectClass *gobject_klass = (GObjectClass *) klass;
  YDataClass *ydata_klass = (YDataClass *) klass;
	YScalarClass *scalar_klass = (YScalarClass *) klass;
  ydata_klass->dup	  = y_scalar_tcp_receiver_dup;
	scalar_klass->get_value	  = y_scalar_tcp_receiver_get_value;
}

static void
y_scalar_tcp_receiver_init(YScalarTcpReceiver *val)
{}

static
gboolean scalar_poll_func(GObject *pollable_stream,gpointer user_data)
{
  YScalarTcpReceiver *r = (YScalarTcpReceiver *) user_data;
  gint buffer[3];
  GError *err = NULL;
  g_pollable_input_stream_read_nonblocking(G_POLLABLE_INPUT_STREAM(pollable_stream),buffer,3*sizeof(gint),NULL,&err);
  if(err!=NULL) {
    g_warning("error during read");
    g_error_free(err);
    return TRUE;
  }
  if(g_ntohl(buffer[0])==r->id) {
    //g_message("hello (%d %d)",g_ntohl(buffer[1]),g_ntohl(buffer[2]));
    g_assert(buffer[1]==0 && buffer[2]==0);
    guint64 i;
    g_pollable_input_stream_read_nonblocking(G_POLLABLE_INPUT_STREAM(pollable_stream),&i,sizeof(guint64),NULL,&err);
    if(err!=NULL) {
      g_warning("error during read2");
      g_error_free(err);
      return TRUE;
    }
    guint64 i2 = GUINT64_FROM_BE(i);
    gpointer id = &i2;
    r->val = *(double *)id;
    y_data_emit_changed(Y_DATA(r));
    g_message("read %f",r->val);
  }
  
  return TRUE;
}

YScalarTcpReceiver *y_scalar_tcp_receiver_new (const gchar *url, guint16 port, guint16 id)
{
  YScalarTcpReceiver *r = g_object_new(Y_TYPE_SCALAR_TCP_RECEIVER,NULL);
  r->id = id;
  GSocketClient *client = g_socket_client_new();
  r->socket_conn = g_socket_client_connect_to_host(client,url,port,NULL,NULL);
  GInputStream *i = g_io_stream_get_input_stream(G_IO_STREAM(r->socket_conn));
  GSource *source = g_pollable_input_stream_create_source (G_POLLABLE_INPUT_STREAM(i), NULL);
  g_source_set_callback(source,scalar_poll_func,r,NULL);
  g_source_attach(source,NULL);
  return r;
}

struct _YVectorTcpReceiver {
	YVector      base;
	GSocketConnection *socket_conn;
        guint16 id;
};

G_DEFINE_TYPE (YVectorTcpReceiver, y_vector_tcp_receiver, Y_TYPE_VECTOR);

static void
y_vector_tcp_receiver_class_init (YVectorTcpReceiverClass *klass)
{
  GObjectClass *gobject_klass = (GObjectClass *) klass;
}

static
gboolean vector_poll_func(GObject *pollable_stream,gpointer user_data)
{
  YVectorTcpReceiver *r = (YVectorTcpReceiver *) user_data;
  gint buffer[3];
  g_pollable_input_stream_read_nonblocking(G_POLLABLE_INPUT_STREAM(pollable_stream),buffer,3*sizeof(gint),NULL,NULL);
  if(g_ntohl(buffer[0])==r->id) {
    //g_message("hello (%d %d)",g_ntohl(buffer[1]),g_ntohl(buffer[2]));
    const int len = buffer[1];
    g_assert(buffer[2]==0);
    guint64 buffer2[len];
    g_pollable_input_stream_read_nonblocking(G_POLLABLE_INPUT_STREAM(pollable_stream),buffer2,len*sizeof(guint64),NULL,NULL);
    int i;
    for(i=0;i<len;i++) {
      guint64 i2 = GUINT64_FROM_BE(buffer2[i]);
      gpointer id = &i2;
      double d = *(double *)id;
    }
  }
  
  return TRUE;
}

static void
y_vector_tcp_receiver_init(YVectorTcpReceiver *val)
{}

YVectorTcpReceiver	*y_vector_tcp_receiver_new      (const gchar *url, guint16 port, guint16 id)
{
  YVectorTcpReceiver *r = g_object_new(Y_TYPE_VECTOR_TCP_RECEIVER,NULL);
  r->id = id;
  GSocketClient *client = g_socket_client_new();
  r->socket_conn = g_socket_client_connect_to_host(client,url,port,NULL,NULL);
  GInputStream *i = g_io_stream_get_input_stream(G_IO_STREAM(r->socket_conn));
  GSource *source = g_pollable_input_stream_create_source (G_POLLABLE_INPUT_STREAM(i), NULL);
  g_source_set_callback(source,vector_poll_func,r,NULL);
  g_source_attach(source,NULL);
  return r;
}

