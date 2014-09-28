#!/bin/bash

# This script calls all the other scripts to launch Wireshark on the WAN of
# the 3 OpenWRTs of of our testbed

sh wireshark_openwrt1.sh eth1 ; 
sh wireshark_openwrt2.sh br-wan ;
sh wireshark_openwrt3.sh eth0