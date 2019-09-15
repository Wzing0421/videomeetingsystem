#!/bin/bash
### BEGIN INIT INFO
# Provides:          bbzhh.com
# Required-Start:  
# Required-Stop: 
# Default-Start:
# Default-Stop:
# Short-Description: 
# Description:       Linux ADB
### END INIT INFO
sudo chmod 333 /sys/class/gpio/export
sleep 0.1
sudo echo 39 > /sys/class/gpio/export
sleep 0.1
sudo chmod 666 /sys/class/gpio/gpio39/direction
sudo chmod 666 /sys/class/gpio/gpio39/value
sleep 0.1
sudo echo out > /sys/class/gpio/gpio39/direction
sleep 0.1
sudo echo 1 > /sys/class/gpio/gpio39/value
sleep 0.1 
sudo echo 42 > /sys/class/gpio/export
sleep 0.1
sudo chmod 666 /sys/class/gpio/gpio42/direction
sudo chmod 666 /sys/class/gpio/gpio42/value
sleep 0.1
sudo echo out > /sys/class/gpio/gpio42/direction
sleep 0.1
sudo echo 0 > /sys/class/gpio/gpio42/value
sleep 0.1
sudo echo 157 > /sys/class/gpio/export
sleep 0.1
sudo chmod 666 /sys/class/gpio/gpio157/direction
sudo chmod 666 /sys/class/gpio/gpio157/value
sleep 0.1
sudo echo out > /sys/class/gpio/gpio157/direction
sleep 0.1
sudo echo 0 > /sys/class/gpio/gpio157/value 
sleep 0.1
sudo ifconfig eth0 192.168.0.101  netmask 255.255.255.0
sudo route add default gw 192.168.0.1
sleep 1
sudo killall myrecv mychdet myvideo python vlc
sleep 0.1
cd /home/firefly/Desktop/videomeet/web
sudo chmod a+x setip
./setip
python server.py >/dev/null &
sleep 1
cd /home/firefly/Desktop/videomeet/ChannelDetection
chmod 777 gpioModule.sh
sleep 0.1
chmod a+x mychdet
./mychdet > /dev/null &
sleep 0.1
cd /home/firefly/Desktop/videomeet/UDPRecv
chmod a+x myrecv
./myrecv > /dev/null &
sleep 0.1
cd /home/firefly/Desktop/videomeet/UDPVideo
chmod a+x myvideo
./myvideo > /dev/null &
sleep 3
cd /home/firefly/Desktop/videomeet/check_encoder
chmod 777 gpioModule.sh
sleep 0.1
chmod a+x ch_encoder
./ch_encoder > /dev/null &
sleep 3
vlc udp://@:10002
