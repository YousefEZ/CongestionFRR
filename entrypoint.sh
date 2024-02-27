#!/bin/bash
echo "Simulating ns3 on file: $1"

cp user_src/* scratch/

./ns3 build
./ns3 run "scratch/$1"

