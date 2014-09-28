#!/bin/sh

# TEMPLATE script for launching wireshark on the xTR and reading the traffic from local host

# $1 = interface name

mkdir /tmp/pipes ;
rm /tmp/pipes/openwrt_$1 ;
mkfifo /tmp/pipes/openwrt_$1 ;

sshpass -p 'ROUTER_PASSWORD' ssh root@ROUTER_ADDRESS 'tcpdump -s 0 -U -n -w - -i '$1' not port 22' > /tmp/pipes/openwrt_$1 &
echo 'HOST_PASSWORD' | sudo -S wireshark -k -i /tmp/pipes/openwrt_$1  &

echo '...Capturing '$1' on /tmp/pipes/openwrt_'$1 ;
