#!/bin/bash

# Define parameters
bandwidth_bottleneck="150kbps"
bandwidth_access="600kbps"
bandwidth_udp_access="100kbps"
delay_bottleneck="20ms"
delay_access="20ms"
delay_alternate="100ms"
bandwidth_alternate="600kbps"

# Define initial test number
test_number=16

# Define function to run experiments for a given policy number
run_experiment() {
	local policy_number=$1
	local test_number=$2

	# Run experiments with current policy number
	./run_experiments.sh "$test_number" "$bandwidth_bottleneck" "$bandwidth_access" "$bandwidth_udp_access" "$delay_bottleneck" "$delay_access" "$delay_alternate" "$bandwidth_alternate" "$policy_number"
}

# Loop over policy numbers and run experiments in parallel
for policy_number in 20 40 60 80 99; do
	# Run experiments in parallel
	run_experiment "$policy_number" "$test_number" &

	# Increment test number
	((test_number++))
done

# Wait for all background processes to finish
wait
