#include <libgssdp/gssdp.h>
#include <gio/gio.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#include <stdlib.h>

int main(int argc, char **argv)
{
#if !GLIB_CHECK_VERSION (2, 35, 0)
    g_type_init ();
#endif
    
    gst_init(&argc, &argv);

    /*GError *error;
    GSSDPClient *client = g_initable_new(GSSDP_TYPE_CLIENT, NULL, &error, NULL);
    if (error) {
        g_printerr("Error creating the GSSDP client: %s\n", error->message);
        g_error_free(error);
        return EXIT_FAILURE;
    }

    GSSDPResourceGroup *resource_group = gssdp_resource_group_new (client);
    gssdp_resource_group_add_resource_simple
        (resource_group,
         "upnp:rootdevice",
         "uuid:1234abcd-12ab-12ab-12ab-1234567abc12::upnp:rootdevice",
         "http://192.168.1.100/");
         gssdp_resource_group_set_available(resource_group, TRUE);*/

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    GstRTSPServer *server = gst_rtsp_server_new();
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);

    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory,
                                      "( v4l2src device=/dev/video0 "
                                      "! omxh264enc "
                                      "! video/x-h264,width=720,height=480,framerate=25/1,profile=high,target-bitrate=8000000 "
                                      "! h264parse "
                                      "! rtph264pay name=pay0 config-interval=1 pt=96 )");

    gst_rtsp_mount_points_add_factory(mounts, "/camera", factory);
    g_object_unref(mounts);

    gst_rtsp_server_attach(server, NULL);

    /* start serving */
    g_print("stream ready at rtsp://127.0.0.1:8554/camera\n");
    g_main_loop_run(loop);

    // done
    g_main_loop_unref(loop);
    g_object_unref(resource_group);
    g_object_unref(client);
    
    return 0;
}
