#include <math.h>
#include <y-data-simple.h>
#include <y-data-tcp.h>

YData *d1, *d2;
static int index = 0;

static gboolean change_data_timer (gpointer user_data)
{
  YScalarVal *s = (YScalarVal*) user_data;
  double *v = y_scalar_val_get_val(s);
  index++;
  *v = (double) index;
  y_data_emit_changed(Y_DATA(s));
  g_message("emit %d",index);
  return TRUE;
}

static gboolean change_data_timer2 (gpointer user_data)
{
  YVectorVal *s = (YVectorVal*) user_data;
  double *v = y_vector_val_get_array(s);
  index++;
  *v = (double) index;
  y_data_emit_changed(Y_DATA(s));
  g_message("emit %d",index);
  return TRUE;
}

int
main (int argc, char *argv[])
{
  d1 = y_scalar_val_new(0.0);
  double a[10];
  int i;
  for(i=0;i<10;i++) {
    a[i]=(double) i;
  }
  d2 = y_vector_val_new_copy(a,10);
  
  y_data_tcp_server_init (60000);
  
  YDataTcpSender *s = y_data_tcp_sender_new (d2, 1);
  
  g_timeout_add(1000, change_data_timer2, d2);
  
  //YScalarTcpReceiver *r = y_scalar_tcp_receiver_new("localhost",60000,1);
  
  GMainLoop *loop = g_main_loop_new(NULL,FALSE);
  g_main_loop_run(loop);

  return 0;
}
