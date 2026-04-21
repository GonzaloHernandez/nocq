#!/bin/bash

GAME="rand.gm"
EXE="./nocq"
PARAMS="10 5 1 5"

echo "--- Step 0: Generating Game ---"
$EXE --rand $PARAMS --export-gm $GAME

echo "--- step 1: extracting full ground truth (zra) ---"
# store the output
ZRA_FULL_OUT=$($EXE --gm $GAME --zra --print-solution)

# extract the sets
EVEN_SET=$(echo "$ZRA_FULL_OUT" | grep "EVEN {" | sed -n 's/.*{\(.*\)}.*/\1/p' | tr ',' ' ')
ODD_SET=$(echo "$ZRA_FULL_OUT" | grep "ODD  {" | sed -n 's/.*{\(.*\)}.*/\1/p' | tr ',' ' ')

echo "ZRA EVEN: { $EVEN_SET }"
echo "ZRA ODD:  { $ODD_SET }"
echo "--------------------------------------------------"

PASS=0
FAIL=0

# function to run the check
verify_node() {
    local node=$1
    local expected=$2
    echo -n "node $node (expected $expected): "
    
    RESULT=$($EXE --gm $GAME --noc --chuffed --init $node)
    
    if [[ $RESULT == *"$node: $expected"* ]]; then
        echo "✅ match"
        ((PASS++))
    else
        echo "❌ mismatch! result: $RESULT"
        ((FAIL++))
    fi
}

# check all even nodes
for n in $EVEN_SET; do
    verify_node "$n" "EVEN"
done

# check all odd nodes
for n in $ODD_SET; do
    verify_node "$n" "ODD"
done

echo "--------------------------------------------------"
echo "verification complete: $PASS passed, $FAIL failed."