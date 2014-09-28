#!/bin/sh

# Local testbed

mkdir /tmp/pipes ;
rm /tmp/pipes/ubuntu2_$1 ;
mkfifo /tmp/pipes/ubuntu2_$1 ;

sshpass -p '***' ssh root@192.168.1.152 'tcpdump -s 0 -U -n -w - -i '$1' not port 22' > /tmp/pipes/ubuntu2_$1 &
echo '***' | sudo -S wireshark -k -i /tmp/pipes/ubuntu2_$1  &
