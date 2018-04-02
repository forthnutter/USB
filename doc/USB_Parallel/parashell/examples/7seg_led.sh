#!/bin/sh
#####################################################
# 7seg_led.sh -- brettcarroll@wirefire.com
# 7 segment LED module script
#####################################################

send_data="/usr/local/bin/parashell 0x378 "

REPEAT=10	# Number of times to loop
DELAY=100000  	# Set the blink delay in microseconds

loop=1

while [ "$loop" -le "$REPEAT" ]
do
    for data_out in 1 2 4 8 16 32 64 8 1 3 7 71 103 119 127 119 127 119
    do
    $send_data $data_out
#    echo $data_out     # Display current value being sent
	usleep $DELAY
    done

    loop=$((loop+1))
done

echo

