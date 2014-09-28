#!/bin/sh

mkdir /tmp/pipes ;
rm /tmp/pipes/upchost_$1 ;
mkfifo /tmp/pipes/upchost_$1 ;

sshpass -p '***' ssh root@***.46 'tcpdump -s 0 -U -n -w - -i '$1' not port 22' > /tmp/pipes/upchost_$1 &
echo '***' | sudo -S wireshark -k -i /tmp/pipes/upchost_$1  &
