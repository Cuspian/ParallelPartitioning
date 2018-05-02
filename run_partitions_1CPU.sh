#!/bin/bash
####################################
#
# Evaluate partitions
#
####################################

# Running partitions for sets with 4 to 10 elements
# with 1 CPU

for n in {4..10}
do
    echo "Generating costs n = $n"
    ./CostGenerator "$n"

    echo "$n partitions $i CPUs.txt"
    ./SetPartition1CPU > "$n""1CPU.txt"

done


