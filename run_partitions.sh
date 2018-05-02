#!/bin/bash
####################################
#
# Evaluate partitions
#
####################################

# Running partitions for sets with 4 to 10 elements
# with 2 to 12 CPUs

for n in {4..10}
do
    echo "Generating costs n = $n"
    ./CostGenerator "$n"

    for i in {2..12};
    do
        echo "$n partitions $i CPUs.txt"
        mpirun -np $i ./SetPartitionNCPUs > "$n""$i""CPU.txt"
    done

done


