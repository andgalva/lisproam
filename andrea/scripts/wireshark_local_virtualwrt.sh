#!/bin/sh

# Local testbed

mkdir /tmp/pipes ;
rm /tmp/pipes/virtualwrt_$1 ;
mkfifo /tmp/pipes/virtualwrt_$1 ;

sshpass -p '***' ssh root@192.168.1.153 'tcpdump -s 0 -U -n -w - -i '$1' not port 22' > /tmp/pipes/virtualwrt_$1 &
echo '***' | sudo -S wireshark -k -i /tmp/pipes/virtualwrt_$1  &
