#!/bin/sh

echo "User Info" >> log.txt
cat /etc/passwd | grep ashish>> log.txt

echo "OS Type/Brand" >> log.txt
uname -o >> log.txt

echo "OS Distribution:" >> log.txt
lsb_release -s -i >> log.txt

echo "OS version:" >> log.txt
lsb_release -s -r >> log.txt

echo "Kernel Version, build and build time:" >> log.txt
cat /proc/version >> log.txt

echo "System Architecture info:" >> log.txt
lscpu >> log.txt

echo "Info on File System Memory" >> log.txt
df -h >> log.txt

exit 0

