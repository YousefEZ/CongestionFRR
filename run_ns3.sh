#!/bin/bash

echo "Running ns3 script"

mkdir -p traces/delay-primary/1ms/20/frr
./ns3 run 'scratch/combined-frr.cc --delay_primary=1ms --policy_threshold=20 --dir=traces/delay-primary/1ms/20/frr/'

echo "NS3 script finished"
