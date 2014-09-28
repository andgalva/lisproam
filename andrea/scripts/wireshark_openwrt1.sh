#!/bin/sh

mkdir /tmp/pipes ;
rm /tmp/pipes/openwrt1_$1 ;
mkfifo /tmp/pipes/openwrt1_$1 ;

sshpass -p '***' ssh root@***.44 'tcpdump -s 0 -U -n -w - -i '$1' not port 22' > /tmp/pipes/openwrt1_$1 &
echo '***' | sudo -S wireshark -k -i /tmp/pipes/openwrt1_$1  &