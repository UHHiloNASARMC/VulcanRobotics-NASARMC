#!/bin/bash

IP=`/sbin/ifconfig eth0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'`
function pause(){
   read -p "$*"
}


echo Pi IP Address is $IP

pause "Press [enter] key to continue..."



raspivid -t 0 -h 720 -w 1080 -fps 25 -hf -b 2000000 -o - | gst-launch-1.0 -v fdsrc ! h264parse !  rtph264pay config-interval=1 pt=96 ! gdppay ! tcpserversink host=$IP port=9000




## alternative cmd
#gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-h264,width=1280,height=720,framerate=30/1 ! h264parse ! rtph264pay pt=127 config-interval=4 ! udpsink host=$1 port=9000

#raspivid -t 0 -h 720 -w 1080 -fps 25 -hf -b 2000000 -o - | gst-launch-1.0 -v fdsrc ! h264parse !  rtph264pay config-interval=1 pt=96 ! gdppay ! udpsink host=$1 port=9000

#gst-launch-1.0 v4l2src ! video/x-raw-yuv,width=128,height=96,format='(fourcc)'UYVY ! ffmpegcolorspace ! ffenc_h263 ! video/x-h263 ! rtph263ppay pt=96 ! udpsink host=$1 port=5000 sync=false




exit 0

echo on the client side, run:
echo gst-launch-1.0 -v tcpclientsrc host=$1 port=9000  ! gdpdepay !  rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false


