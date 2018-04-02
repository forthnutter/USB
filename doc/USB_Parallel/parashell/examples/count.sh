#!/bin/sh
#####################################################
# count.sh -- brettcarroll@wirefire.com
# 7 segment LED module script
#####################################################

send_data="/usr/local/bin/parashell 0x378 "
send_control="/usr/local/bin/parashell 0x37a "

REPEAT=100
DELAY=100000	#Delay in microseconds

loop=1

while [ "$loop" -le "$REPEAT" ]
do
    for data_out in 68 107 110 92 62 31 100 127 124 119 0 0
    do
	for dot in 4 0
	do
    /usr/bin/pouta $dot
    $send_data $data_out
#    echo $data_out     # Display current value being sent
	usleep $DELAY
	$send_control $dot

    done
    done
    loop=$((loop+1))
done

echo

