#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <libgssdp/gssdp.h>
#include <gio/gio.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>


void GetDeviceIPV4Address(char ** const address_out)
{
  int fd;
  struct ifreq ifr;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);

  char *address = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
  *address_out = strdup(address);
}


int main(int argc, char **argv)
{
    gst_init(&argc, &argv);
    
    char *address = NULL;
    GetDeviceIPV4Address(&address);

    GMainContext *main_context = g_main_context_new();

    GError *error = NULL;
    GSSDPClient *client = gssdp_client_new(main_context, NULL, &error);
    if (error) {
      g_printerr("Error creating the GSSDP client: %s\n", error->message);
      g_error_free(error);
      return EXIT_FAILURE;
    }

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    GstRTSPServer *server = gst_rtsp_server_new();
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);

    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
    
    /*gst_rtsp_media_factory_set_launch(factory,
                                      "( videotestsrc "
                                      "! omxh264enc "
                                      "! video/x-h264,width=720,height=480,framerate=25/1,profile=high,target-bitrate=8000000 "
                                      "! h264parse "
                                      "! rtph264pay name=pay0 config-interval=1 pt=96 )");*/
    gst_rtsp_media_factory_set_launch(factory,
                                      "( rpicamsrc bitrate=1000000 "
                                      "! h264parse "
                                      "! rtph264pay name=pay0 config-interval=1 pt=96 )");

    gst_rtsp_mount_points_add_factory(mounts, "/camera", factory);
    g_object_unref(mounts);

    gst_rtsp_server_attach(server, NULL);

    int server_port = gst_rtsp_server_get_bound_port(server);
    gchar *server_addr = gst_rtsp_server_get_address(server);

    char rtsp_address[128];
    memset(rtsp_address, 0, sizeof(rtsp_address));
    strncpy(rtsp_address, "rtsp://", 7);
    strncpy(rtsp_address+7, address, strlen(address));
    strncpy(rtsp_address+7+strlen(address), ":8554/camera", 12);
    free(address);

    GSSDPResourceGroup *resource_group = gssdp_resource_group_new (client);
    gssdp_resource_group_add_resource_simple
      (resource_group,
       "upnp:RpiCamera",
       "uuid:1234abcd-12ab-12ab-12ab-1234567abc12::upnp:RpiCamera",
       rtsp_address);
    gssdp_resource_group_set_available(resource_group, TRUE);

    // start
    g_print("RTSP Rpi Camera - stream ready at rtsp://%s:%i/camera\n", server_addr, server_port);
    g_main_loop_run(loop);

    // dealloc loop
    g_main_loop_unref(loop);

    // ssdp client
    g_object_unref(resource_group);
    g_object_unref(client);
    g_object_unref(main_context);
    
    return 0;
}
