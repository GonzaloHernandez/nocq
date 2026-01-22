#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 <command>"
    exit 1
fi

while true; do
    output=$("$@")

    if [[ "$output" != "." ]]; then
        echo "$output"
        break
    fi

    sleep 0.1
done
