#!/bin/bash

# Define parameters
bandwidth_bottleneck="150Kbps"
bandwidth_access="600kbps"
bandwidth_udp_access="100kbps"
delay_bottleneck="20ms"
delay_access="20ms"
delay_alternate="20ms"
bandwidth_alternate="600kbps"

# Define function to run experiments for a given policy number
run_experiment() {
	local policy_number=$1
	local test_name=$2
	local test_variable=$3

	# Run experiments with current policy number
	./run_experiments.sh "$test_name" "$bandwidth_bottleneck" "$bandwidth_access" "$bandwidth_udp_access" "$delay_bottleneck" "$delay_access" "$delay_alternate" "$bandwidth_alternate" "$policy_number" "$test_variable"
}

# Define initial test number
test_name="delay-primary"

# Define delay_vals array correctly
delay_vals=("0ms" "40ms" "60ms" "80ms")

for delay_val in "${delay_vals[@]}"; do
	delay_bottleneck=$delay_val
	for policy_number in 20 40 60 80 99; do
		run_experiment "$policy_number" "$test_name" "$delay_val"
	done
done
delay_bottleneck="20ms"

test_name="bandwidth-alternate"
# Define bandwidth_vals array correctly
bandwidth_vals=("200Kbps" "400Kbps" "600Kbps" "800Kbps" "1000Kbps")

for bandwidth_val in "${bandwidth_vals[@]}"; do
	bandwidth_alternate=$bandwidth_val
	for policy_number in 20 40 60 80 99; do
		run_experiment "$policy_number" "$test_name" "$bandwidth_val"
	done
done
bandwidth_alternate="600kbps"

test_name="delay-alternate"
# Define delay_vals array correctly
delay_vals=("0ms" "20ms" "40ms" "60ms" "80ms" "100ms")

for delay_val in "${delay_vals[@]}"; do
	delay_alternate=$delay_val
	for policy_number in 20 40 60 80 99; do
		run_experiment "$policy_number" "$test_name" "$delay_val"
	done
done
delay_alternate="20ms"
