#!/bin/bash
echo "Simulating ns3 on file: $1"

cp user_src/* scratch/

export NS_LOG=FRRQueue=level_all

./ns3 build 
./ns3 run "scratch/$1" > traces/output.log 2>&1

