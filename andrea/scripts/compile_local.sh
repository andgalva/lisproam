# For local testbed
# Compiles and updates lispd on our two servers we use are xTRs

cd ~/workspace/lisproam ;
make ;

echo "";
echo "-> Copying to Ubuntu Server 1..." ;

sshpass -p '***' scp ~/workspace/lisproam/lispd root@192.168.1.151:. ;

echo "";
echo "-> Copying to Ubuntu Server 2..." ;

sshpass -p '***' scp ~/workspace/lisproam/lispd root@192.168.1.152:. ;

