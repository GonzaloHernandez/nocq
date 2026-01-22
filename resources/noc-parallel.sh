#!/bin/bash

if [ $# -eq 0 ]; then
    ./noc --help
    exit 1
fi

user_args=("$@")

./noc --noc-even "${user_args[@]}" & PID1=$!

./noc --noc-odd "${user_args[@]}" & PID2=$!

wait -n

kill $PID1 $PID2 2>/dev/null
