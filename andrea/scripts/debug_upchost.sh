#!/bin/sh

# For local testbed, opens a tab for each host (CN and mobile host)

konsole --noclose --new-tab -e sshpass -p '***' ssh andrea@192.168.56.101 ;
konsole --noclose --new-tab -e sshpass -p '***' ssh root@192.168.56.160 ;