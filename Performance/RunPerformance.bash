#!/bin/sh

#  RunPerformance.bash
#  FlashRadixTree
#
#  Created by Matthew Varendorff on 15/4/2024.
#


# check arguments. Takes binary name but defaults to ../build/FlashRadixTree
if [ $# -eq 0 ]
then
    BINARY="../build/FlashRadixTreeWithSplay"
else
    BINARY=$1
fi

# check if binary exists
if [ ! -f $BINARY ]
then
    echo "Binary not found"
    exit 1
fi

# check if binary is executable
if [ ! -x $BINARY ]
then
    echo "Binary not executable"
    exit 1
fi

# check if binary is a file
if [ ! -f $BINARY ]
then
    echo "Binary not a file"
    exit 1
fi

SAMPLE_FILE="../sample_data.txt"

#clear benchmark folder contents
rm benchmark/*

#run binary increasing multiplier by 50 earch time unitll 300
#format $BINARY -f $SAMPLE_FILE -m $i -t > benchmark/multiplier_$i.txt
for ((i = 1; i <= 301; i += 50));
do
echo "Running $BINARY -f $SAMPLE_FILE -m $i -t > benchmarks/multiplier_$i.txt"

$BINARY -f $SAMPLE_FILE -m $i -t > benchmarks/multiplier_$i.txt
done
