#!/bin/bash

if [ $# -eq 0 ]; then
    ./noc --help
    exit 1
fi

user_args=("$@")

# Launch even instance in background and get PID
./nocq --noc-even "${user_args[@]}" & 
PID1=$!
echo "PID process (EVEN) : $PID1"

# Launch odd instance in background and get PID
./nocq --noc-odd "${user_args[@]}" & 
PID2=$!
echo "PID process (ODD)  : $PID2"

# Explicitly wait for one of these specific PIDs to finish
wait -n $PID1 $PID2

# Check which one finished by probing if they are still alive
kill -0 $PID1 2>/dev/null
EVEN_ALIVE=$?

kill -0 $PID2 2>/dev/null
ODD_ALIVE=$?

echo "--------------------------------------------------"
if [ $EVEN_ALIVE -ne 0 ] && [ $ODD_ALIVE -eq 0 ]; then
    echo "🏆 Process EVEN ($PID1) finished first!"
elif [ $ODD_ALIVE -ne 0 ] && [ $EVEN_ALIVE -eq 0 ]; then
    echo "🏆 Process ODD ($PID2) finished first!"
else
    # In the rare event both finished at the exact same fraction of a second
    echo "👔 Both processes finished at virtually the same time."
fi
echo "--------------------------------------------------"

# Clean up the remaining process
kill $PID1 $PID2 2>/dev/null

echo "Done."