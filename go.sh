#!/bin/bash
#git pull 
clear
make clean
make

scp ptp4l user@192.168.0.119:/home/user
scp ptp4l user@192.168.0.120:/home/user
#scp ptp4l user@192.168.0.102:/home/user
#scp ptp4l user@192.168.0.103:/home/user
#scp ptp4l user@192.168.0.102:/home/user

#echo "-i ens33 -S -m -w"
#gdb ./ptp4l
./ptp4l -i ens33 -S -m -w
