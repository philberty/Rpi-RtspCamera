FROM resin/rpi-raspbian

MAINTAINER Philip Herron <herron.philip@googlemail.com>

RUN apt-get update
RUN apt-get install gstreamer1.0 gstreamer1.0-tools gstreamer1.0-omx libgstreamer1.0-dev libgssdp-1.0-dev v4l-utils openssh-server wget build-essential

RUN wget https://gstreamer.freedesktop.org/src/gst-rtsp-server/gst-rtsp-server-1.4.4.tar.xz
RUN tar xvf gst-rtsp-server-1.4.4.tar.xz; cd gst-rtsp-server-1.4.4; ./configure --prefix=/usr; make; make install; cd ..

# Enable systemd init system in container
ENV INITSYSTEM on

EXPOSE 8554

COPY . /usr/src/app
WORKDIR /usr/src/app

RUN gcc -g -O2 -Wall `pkg-config --cflags --libs gstreamer-rtsp-server` RpiCameraRtspServer.c -o RpiCameraRtspServer

CMD ["bash", "start.sh"]
