#!/bin/sh

# DBG="-d z"
DBG=""

TC_NORMAL=(\
"-ep hw3t1 0 -ep hw3t2 0 -ee $DBG" \
"-ep hw3t1 50 -ep hw3t2 50 -ee $DBG" \
"-ep hw3t1 50 -ep hw3t2 90 -ee $DBG" \
"-ep hw3t1 100 -ep hw3t2 100 -ee $DBG" \
"-ep hw3t1 40 -ep hw3t2 55 -ee $DBG" \
"-ep hw3t1 40 -ep hw3t2 90 -ee $DBG" \
"-ep hw3t1 90 -ep hw3t2 100 -ee $DBG" \
"-ep hw3t1 60 -ep hw3t3 50 -ee $DBG" \
)

TIMEOUT="timeout 1s"

# Start testing
echo -e "===== Start testcases ====="
for ((i=0; i<${#TC_NORMAL[@]}; i++)); do
    echo -e "===== Test case $(($i+1)): ${TC_NORMAL[$i]} ====="
    $TIMEOUT ../build.linux/nachos ${TC_NORMAL[$i]}
done

exit 0
