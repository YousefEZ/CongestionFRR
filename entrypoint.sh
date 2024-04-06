#!/bin/bash

rm -rf traces/
mkdir traces

cp user_src/* scratch/
export NS_LOG=FRRQueue=level_all

cp ./run_ns3.sh ./.

./ns3 build

if [ $? -ne 0 ]; then
	echo "ns3 build failed"
	exit 1
fi

if [ -z "$1" ]; then
	exit 0
fi

# ./ns3 run 'scratch/combined-frr.cc --delay_bottleneck=1ms'
./run_ns3.sh
if [ $? -ne 0 ]; then
	echo "./ns3 run for $name failed"
	exit 1
fi
exit 0
