#!/bin/bash
#git pull 
clear
make clean
make

<<<<<<< HEAD
#scp ptp4l user@192.168.0.119:/home/user
#scp ptp4l user@192.168.0.120:/home/user
#scp ptp4l user@192.168.0.121:/home/user
=======
#scp ptp4l user@192.168.0.111:/home/user
#scp ptp4l user@192.168.0.105:/home/user
#scp ptp4l user@192.168.0.102:/home/user
#scp ptp4l user@192.168.0.103:/home/user
>>>>>>> 125f1c59c744b519235a7dd1ff810433b6eed85f
#scp ptp4l user@192.168.0.104:/home/user
#scp ptp4l user@192.168.0.103:/home/user



#scp ptp4l user@192.168.0.102:/home/user

#echo "-i ens33 -S -m -w"
#gdb ./ptp4l
./ptp4l -i ens33 -S -m -w
