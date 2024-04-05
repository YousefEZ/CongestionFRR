#!/bin/bash

echo "Running ns3 script"

run_experiment() {
	local test_variable=$1
	local test_value=$2
	local policy_threshold=$3
	local dir="traces/$test_variable/$test_value/$policy_threshold/frr/"
	mkdir -p "$dir"

	./ns3 run "scratch/combined-frr.cc --$test_variable=$test_value --policy_threshold=$policy_threshold --dir=$dir"
}

# Bandwidth primary experiments
echo "Bandwidth primary_experiments"
delay_vals=("0ms" "10ms" "20ms" "30ms" "40ms" "60ms" "80ms" "100ms")
for delay_val in "${delay_vals[@]}"; do
	echo "Delay value: $delay_val"
	for policy_number in 20 40 60 80 99; do
		run_experiment "delay_primary" "$delay_val" "$policy_number" &
	done
done

wait
echo "NS3 script finished"
