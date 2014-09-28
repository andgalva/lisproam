#!/bin/sh

# For global testbed
# 1 - Copies workspace/lisproam/ (also Makefile) content in the OpenWRT build fodler

echo "";
echo "-> Copying *.c, *.h and Makefile to destination folder..." ;

cp -r ~/workspace/lisproam/*.c ~/openwrt/trunk/build_dir/target-mips_34kc_uClibc-0.9.33.2/lispmob-0.3/lispd/ ;
rm -r ~/openwrt/trunk/build_dir/target-mips_34kc_uClibc-0.9.33.2/lispmob-0.3/lispd/andrea ;
mkdir ~/openwrt/trunk/build_dir/target-mips_34kc_uClibc-0.9.33.2/lispmob-0.3/lispd/andrea ;
cp -r ~/workspace/lisproam/andrea/* ~/openwrt/trunk/build_dir/target-mips_34kc_uClibc-0.9.33.2/lispmob-0.3/lispd/andrea ;
cp -r ~/workspace/lisproam/*.h ~/openwrt/trunk/build_dir/target-mips_34kc_uClibc-0.9.33.2/lispmob-0.3/lispd/ ;
cp -r ~/workspace/lisproam/Makefile ~/openwrt/trunk/build_dir/target-mips_34kc_uClibc-0.9.33.2/lispmob-0.3/lispd/ ;

# 2 - Compiles from the OpenWRT build folder

echo "";
echo "-> Compiling..." ;

cd ~/openwrt/trunk ;
make ./package/feeds/packages/lispmob/compile V=s  ;
cd ../.. ;

# 3 - Copies updated lispd to the 2 physical OpenWRTs of the testbed
# We keep the same version of lispd on the virtual OpenWRT (it does not care about handovers)

echo "";
echo "-> Copying to OpenWRT 1..." ;

sshpass -p '***' scp /home/andrea/openwrt/trunk/staging_dir/target-mips_34kc_uClibc-0.9.33.2/root-ar71xx/usr/sbin/lispd root@***.44:. ;

echo "";
echo "-> Copying to OpenWRT 2..." ;

sshpass -p '***' scp /home/andrea/openwrt/trunk/staging_dir/target-mips_34kc_uClibc-0.9.33.2/root-ar71xx/usr/sbin/lispd andrea@***43:. ;

echo "";
echo "-> OK!" ;


