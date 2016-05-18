#!/usr/bin/env bash
set -e

modprobe bcm2835-v4l2
./RpiCameraRtspServer
