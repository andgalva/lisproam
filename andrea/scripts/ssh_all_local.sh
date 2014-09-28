# For local testbed. Opens a tab for each.

# UBUNTU SERVER 1
konsole --noclose --new-tab -e sshpass -p '***' ssh root@192.168.1.151 ;
# UBUNTU SERVER 2
konsole --noclose --new-tab -e sshpass -p '***' ssh root@192.168.1.152 ;
# VIRTUAL OPENWRT
konsole --noclose --new-tab -e sshpass -p '***' ssh root@192.168.1.153 ;
