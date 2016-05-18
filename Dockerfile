FROM resin/rpi-raspbian

MAINTAINER Philip Herron <herron.philip@googlemail.com>

RUN apt-get update
RUN apt-get install gstreamer1.0 gstreamer1.0-tools gstreamer1.0-omx libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgssdp-1.0-dev v4l-utils wget ca-certificates build-essential

RUN wget https://gstreamer.freedesktop.org/src/gst-rtsp-server/gst-rtsp-server-1.4.4.tar.xz
RUN tar xvf gst-rtsp-server-1.4.4.tar.xz; cd gst-rtsp-server-1.4.4; ./configure --prefix=/opt; make; make install; cd ..

ENV PKG_CONFIG_PATH /opt/lib/pkgconfig:/usr/lib/pkgconfig:$PKG_CONFIG_PATH
ENV LD_LIBRARY_PATH /opt/lib:$LD_LIBRARY_PATH
ENV INITSYSTEM on

EXPOSE 8554

COPY . /usr/src/app
WORKDIR /usr/src/app

RUN gcc -g -O2 -Wall `pkg-config --cflags --libs gstreamer-rtsp-server gssdp-1.0` RpiCameraRtspServer.c -o RpiCameraRtspServer

CMD ["bash", "start.sh"]
