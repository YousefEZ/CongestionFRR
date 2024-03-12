#!/bin/bash

rm -rf traces/
mkdir traces

cp user_src/* scratch/
export NS_LOG=FRRQueue=level_all

./ns3 build

if [ $? -ne 0 ]; then
  echo "ns3 build failed"
  exit 1
fi

if [ -z "$1" ]; then
  exit 0
fi

./ns3 run "scratch/$1"
if [ $? -ne 0 ]; then
  echo "./ns3 run for $name failed"
  exit 1
fi
exit 0
