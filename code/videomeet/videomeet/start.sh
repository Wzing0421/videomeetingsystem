#!/bin/bash

cd /home/firefly/Desktop/SatRecvDriver/CellSplitter
./CellSplitter > /dev/null &
echo "CellSplitter Start Successful"
sleep 1
cd /home/firefly/Desktop/SatRecvDriver/VideoReceiver
./VideoReceiver > /dev/null &
echo "VideoReceiver Start Successful"
sleep 1
cd /home/firefly/Desktop/UDPRecv
./myrecv >/dev/null &
echo "myrecv Start Successful"
#sleep 1
#vlc udp://@:10002
sleep 2
cd /home/firefly/Desktop/UDPVideo
./myvideo >/dev/null &
sleep 1
echo "myvideo Start Successful"
vlc udp://@:10002 >/dev/null &
