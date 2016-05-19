#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#define VERSION 1

int main(int argc, char **argv)
{    
    gst_init(&argc, &argv);

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    GstRTSPServer *server = gst_rtsp_server_new();
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);

    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory,
                                      "( videotestsrc "
                                      "! omxh264enc "
                                      "! video/x-h264,width=720,height=480,framerate=25/1,profile=high,target-bitrate=8000000 "
                                      "! h264parse "
                                      "! rtph264pay name=pay0 config-interval=1 pt=96 )");    

    gst_rtsp_mount_points_add_factory(mounts, "/camera", factory);
    g_object_unref(mounts);

    gst_rtsp_server_attach(server, NULL);

    int server_port = gst_rtsp_server_get_bound_port(server);
    gchar *server_addr = gst_rtsp_server_get_address(server);
    
    g_print("RTSP %i - stream ready at rtsp://%s:%i/camera\n", VERSION, server_addr, server_port);
    g_main_loop_run(loop);

    g_main_loop_unref(loop);
    
    return 0;
}
