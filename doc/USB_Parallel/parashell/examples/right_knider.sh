#!/bin/sh
#################################################################
# right_knider.sh -- Brett Carroll -- brettcarroll@movlug.com
#
# Purpose: Chasing effect with LEDs
#
# Refer to docs/LED.txt for wiring diagram
#
#################################################################

send_data="/usr/local/bin/parashell 0x378 "

REPEAT=100
DELAY=100000 	# Set the blink delay

loop=1

while [ "$loop" -le "$REPEAT" ]
do
    for data_out in 0 1 2 4 8 16 32 64 128 0 0 128 64 32 16 8 4 2 1 0 0
    do
    $send_data $data_out
#    echo $data_out 	# used for Debugging
    usleep $DELAY

    done

    loop=$((loop+1))

done

echo

exit 0

