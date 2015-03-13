#/bin/bash

#This script is intended to speed up octopi first run. 
#It is called by systemd service "octopi.service"

#First we get the unrequired package list
/usr/bin/pacman -Qt > /dev/null

#Next we get the foreign package list
/usr/bin/pacman -Qm > /dev/null
