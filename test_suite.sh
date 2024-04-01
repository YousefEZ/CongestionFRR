#!/bin/bash

# Define parameters
bandwidth_bottleneck="150kbps"
bandwidth_access="600kbps"
bandwidth_udp_access="100kbps"
delay_bottleneck="20ms"
delay_access="20ms"

# Define initial test number
test_number=1

# Loop over policy numbers
for policy_number in 20 40 60 80 99; do
	# Run experiments with current policy number
	./run_experiments.sh "$test_number" "$bandwidth_bottleneck" "$bandwidth_access" "$bandwidth_udp_access" "$delay_bottleneck" "$delay_access" "$policy_number"

	# Increment test number
	((test_number++))
done

# Update delay parameters
delay_bottleneck="2ms"
delay_access="2ms"

# Loop over policy numbers with updated delay parameters
for policy_number in 20 40 60 80 99; do
	# Run experiments with current policy number and updated delay parameters
	./run_experiments.sh "$test_number" "$bandwidth_bottleneck" "$bandwidth_access" "$bandwidth_udp_access" "$delay_bottleneck" "$delay_access" "$policy_number"

	# Increment test number
	((test_number++))
done
