#!/bin/bash


#gst-launch-1.0 -v udpsrc port=9000 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264' !      rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false

#gst-launch-1.0  udpsrc  port=5000 ! application/x-rtp, clock-rate=90000,payload=96 ! rtph263pdepay queue-delay=0 ! ffdec_h263 ! xvimagesink


if [ -z "$1" ]
    then
        echo Usage: ./client.sh IP_OF_PI_HOST
else

	gst-launch-1.0 -v tcpclientsrc host=$1 port=9000  ! gdpdepay !  rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false

fi


