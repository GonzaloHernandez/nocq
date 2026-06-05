#!/bin/bash

if [ $# -eq 0 ]; then
    ./nocq --help
    exit 1
fi

user_args=("$@")

./nocq --noc-even "${user_args[@]}" > /tmp/nocq_even_$$ 2>/dev/null &
PID1=$!

./nocq --noc-odd "${user_args[@]}" > /tmp/nocq_odd_$$ 2>/dev/null &
PID2=$!

wait -n $PID1 $PID2

kill -0 $PID1 2>/dev/null
EVEN_ALIVE=$?

kill -0 $PID2 2>/dev/null
ODD_ALIVE=$?

if [ $EVEN_ALIVE -ne 0 ]; then
    re=$(cat /tmp/nocq_even_$$)
    if [ "$re" = "EVEN" ]; then
        printf "REALIZABLE\n"
    elif [ "$re" = "ODD" ]; then
        printf "UNREALIZABLE\n"
    else
        printf "ERROR\n"
    fi
elif [ $ODD_ALIVE -ne 0 ]; then
    ro=$(cat /tmp/nocq_odd_$$)
    if [ "$ro" = "EVEN" ]; then
        printf "REALIZABLE\n"
    elif [ "$ro" = "ODD" ]; then
        printf "UNREALIZABLE\n"
    else
        printf "ERROR\n"
    fi
fi

kill $PID1 $PID2 2>/dev/null
rm -f /tmp/nocq_even_$$ /tmp/nocq_odd_$$
