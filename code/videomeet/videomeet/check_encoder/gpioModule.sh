#!/bin/bash
# the first para means the different state of LED
# 0- LED OUT; 1- LED UP; 2-LED twinkle
#the second para means which GPIO is being operated
#there is something wrong with 1_B4 and 1_B0
para="$2"
if [ "$para" = "4_D5" ]; then
num=157
path="/sys/class/gpio/gpio157"
pathval="/sys/class/gpio/gpio157/value"
dir="/sys/class/gpio/gpio157/direction"
echo "157"
elif [ "$para" = "1_A7" ]; then
num=39
path="/sys/class/gpio/gpio39"
pathval="/sys/class/gpio/gpio39/value"
dir="/sys/class/gpio/gpio39/direction"
echo "39"
elif [ "$para" = "1_B0" ]; then
num=40
path="/sys/class/gpio/gpio40"
pathval="/sys/class/gpio/gpio40/value"
dir="/sys/class/gpio/gpio40/direction"
echo "40"
elif [ "$para" = "1_B1" ]; then
num=41
path="/sys/class/gpio/gpio41"
pathval="/sys/class/gpio/gpio41/value"
dir="/sys/class/gpio/gpio41/direction"
echo "41"
elif [ "$para" = "1_B2" ]; then
num=42
path="/sys/class/gpio/gpio42"
pathval="/sys/class/gpio/gpio42/value"
dir="/sys/class/gpio/gpio42/direction"
echo "42"
elif [ "$para" = "1_B4" ]; then
num=44
path="/sys/class/gpio/gpio44"
pathval="/sys/class/gpio/gpio44/value"
dir="/sys/class/gpio/gpio44/direction"
echo "44"
else
echo "wrong GPIO!"
fi
    
if [ -d "$path" ]; then
    if [ $1 -eq "1" ]; then
    sudo echo 1 > "$pathval"
    elif [ $1 -eq "0" ]; then
    sudo echo 0 > "$pathval"
    else
        i=1
	while true
        do
          sudo echo $i > "$pathval"
          i=$(((i+1) % 2))
	  sleep 2
	done
    fi 
else 
    sudo echo $num > /sys/class/gpio/export
    sudo echo out > "$dir"
    if [ $1 -eq "1" ]; then
    sudo echo 1 > "$pathval"
    elif [ $1 -eq "0" ]; then
    sudo echo 0 > "$pathval"
    else
	i=1
        while true
	do 
	  sudo echo $i > "$pathval"
          i=$(((i+1) % 2))
          sleep 2
	done
    fi 
fi
