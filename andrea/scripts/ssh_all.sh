#!/bin/sh

# This script is intended for easily login on mulitple machines in different shell tabs
# Used for global testbed (addresses are hidden)

# OPENWRT 1 (LISP-A wifi)
konsole --noclose --new-tab -e sshpass -p '*** ssh root@8***.44 ;
# OPENWRT 2 (LISP-B wifi)
konsole --noclose --new-tab -e sshpass -p '***' ssh root@***.43 ;
# VIRTUAL OPENWRT (no wifi, used as Correspondent node's network)
konsole --noclose --new-tab -e sshpass -p '***' ssh root@***.57 ;
# UPC HOST (Correspondent node)
konsole --noclose --new-tab -e sshpass -p '***' ssh andrea@***.60 -p 13000 ;
# RADIUS SERVER (domain A)
konsole --noclose --new-tab -e sshpass -p '***' ssh andrea@***.59 ;
# RADIUS SERVER (domain B)
konsole --noclose --new-tab -e sshpass -p '***' ssh andrea@***.58 ;