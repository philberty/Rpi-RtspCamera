#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <libgssdp/gssdp.h>
#include <gio/gio.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static gboolean
remove_func (GstRTSPSessionPool * pool, GstRTSPSession * session,
    GstRTSPServer * server)
{
  return GST_RTSP_FILTER_REMOVE;
}

static gboolean
remove_sessions (GstRTSPServer * server)
{
  GstRTSPSessionPool *pool;

  g_print ("removing all sessions\n");
  pool = gst_rtsp_server_get_session_pool (server);
  gst_rtsp_session_pool_filter (pool,
      (GstRTSPSessionPoolFilterFunc) remove_func, server);
  g_object_unref (pool);

  return FALSE;
}

/* this timeout is periodically run to clean up the expired sessions from the
 * pool. This needs to be run explicitly currently but might be done
 * automatically as part of the mainloop. */
static gboolean
timeout (GstRTSPServer * server, gboolean ignored)
{
  GstRTSPSessionPool *pool;

  pool = gst_rtsp_server_get_session_pool (server);
  gst_rtsp_session_pool_cleanup (pool);
  g_object_unref (pool);

  return TRUE;
}


int main(int argc, char **argv)
{
    gst_init(&argc, &argv);

    GMainContext *main_context = g_main_context_new();

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    GstRTSPServer *server = gst_rtsp_server_new();
    
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory,
                                      "( rpicamsrc bitrate=5000000 "
                                      "! h264parse "
                                      "! rtph264pay name=pay0 config-interval=1 pt=96 )");
    gst_rtsp_media_factory_set_shared(factory, TRUE);
    gst_rtsp_mount_points_add_factory(mounts, "/camera", factory);
    
    int failed = gst_rtsp_server_attach(server, NULL) == 0;
    if (failed)
    {
        g_print("Failed to start rtsp-server");
        return -1;
    }

    /* add a timeout for the session cleanup */
    // g_timeout_add_seconds (2, (GSourceFunc) timeout, server);
    // g_timeout_add_seconds (10, (GSourceFunc) remove_sessions, server);

    int server_port = gst_rtsp_server_get_bound_port(server);
    gchar *server_addr = gst_rtsp_server_get_address(server);

    // start
    g_print("RTSP Feed ready at rtsp://%s:%i/camera\n", server_addr, server_port);
    g_main_loop_run(loop);

    // dealloc loop
    g_main_loop_unref(loop);

    // ssdp client
    g_object_unref(mounts);
    g_object_unref(main_context);
    
    return 0;
}
