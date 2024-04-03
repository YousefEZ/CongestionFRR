#!/bin/bash

# Define parameters
bandwidth_bottleneck="150Kbps"
bandwidth_access="600kbps"
bandwidth_udp_access="100kbps"
delay_bottleneck="20ms"
delay_access="20ms"
delay_alternate="20ms"
bandwidth_alternate="600kbps"

# Define initial test number
test_name="bandwidth-primary"
test_variable="150Kbps"

# Define function to run experiments for a given policy number
run_experiment() {
	local policy_number=$1
	local test_name=$2
	local test_variable=$3

	# Run experiments with current policy number
	./run_experiments.sh "$test_name" "$bandwidth_bottleneck" "$bandwidth_access" "$bandwidth_udp_access" "$delay_bottleneck" "$delay_access" "$delay_alternate" "$bandwidth_alternate" "$policy_number" "$test_variable"
}

# Loop over policy numbers and run experiments in parallel
for policy_number in 20 40 60 80 99; do
	# Run experiments in parallel
	run_experiment "$policy_number" "$test_name" "$test_variable"

	# Increment test number
	((test_number++))
done
