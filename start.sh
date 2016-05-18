#!/usr/bin/env bash
set -e

modprobe bcm2835-v4l2

./RpiCameraRtspServer "( v4l2src device=/dev/video0 ! omxh264enc ! video/x-h264,width=720,height=480,framerate=25/1,profile=high,target-bitrate=8000000 ! h264parse ! rtph264pay name=pay0 config-interval=1 pt=96 )"
