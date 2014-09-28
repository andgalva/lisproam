#!/bin/sh

# For local testbed, opens multiple tabs

konsole --noclose --new-tab -e sshpass -p '***' ssh root@192.168.6.1 
konsole --noclose --new-tab -e sshpass -p '***' ssh root@192.168.6.1 
konsole --noclose --new-tab -e sshpass -p '***' ssh root@192.168.6.1
konsole --noclose --new-tab -e sshpass -p '***' ssh root@192.168.6.1