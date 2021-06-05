#!/bin/sh

pid=$1
location=$(($2))

f=err.dat
dd if=/dev/urandom of=$f count=1 bs=1
err=`cat $f | xxd -i `

echo "Injecting error byte $err to pid=$pid at address $location" >> /tmp/bitflip.log

dd if=$f  of=/proc/$pid/mem count=1 bs=1 seek=$location conv=notrunc

rm $f
