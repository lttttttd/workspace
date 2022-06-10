#!/bin/sh

insmod demo.ko
mknod /dev/ltd c 236 0

#gcc user.c -o user
./user

rmmod demo.ko
