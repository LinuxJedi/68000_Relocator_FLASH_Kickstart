#!/bin/bash

PROJECT=FLASH_KICKSTART
DEVICE=xc9572xl
PART=XC9572XL-10-VQ44

cat <<! >$PROJECT.xst
run
-ifn $PROJECT.v
-ifmt verilog
-ofn $PROJECT.ngc
-p $DEVICE
!
rm -rf xst/work
xst -ifn $PROJECT.xst
RESULT=$?
if [ $RESULT -ne 0 ]; then
    exit $RESULT
fi
ngdbuild -uc $PROJECT.ucf $PROJECT.ngc -p $DEVICE
cpldfit  -p $PART -ofmt vhdl -optimize speed $PROJECT
hprep6 -s IEEE1149 -n $PROJECT -i $PROJECT
hprep6 -s IEEE1532 -n $PROJECT -i $PROJECT
