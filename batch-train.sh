#!/usr/bin/env bash
MAX=1500

for x in `seq $MAX`; do
    echo "Processing $x/$MAX"
    ./neural_trainer
#    cat -n weights.txt
done
