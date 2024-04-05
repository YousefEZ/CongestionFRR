#!/bin/bash

echo "Running ns3 script"

run_experiment() {
	local test_variable=$1
	local test_value=$2
	local policy_threshold=$3
	local dir="traces/$test_variable/$test_value/$policy_threshold"
	mkdir -p "$dir/frr/"
	mkdir -p "$dir/frr-no-udp/"
	mkdir -p "$dir/baseline-udp/"
	mkdir -p "$dir/baseline-no-udp/"

	./ns3 run "scratch/combined-frr.cc --$test_variable=$test_value --policy_threshold=$policy_threshold --dir=$dir/frr/"
	./ns3 run "scratch/combined-frr-no-udp.cc --$test_variable=$test_value --policy_threshold=$policy_threshold --dir=$dir/frr-no-udp/"
	./ns3 run "scratch/combined-frr-no-udp.cc --$test_variable=$test_value --policy_threshold=$policy_threshold --dir=$dir/baseline-no-udp/"
	./ns3 run "scratch/combined-frr-no-udp.cc --$test_variable=$test_value --policy_threshold=$policy_threshold --dir=$dir/baseline-udp/"
}

# Bandwidth primary experiments
echo "Delay Primary experiments"
delay_vals=("0ms" "10ms" "20ms" "30ms" "40ms" "60ms" "80ms" "100ms")
for delay_val in "${delay_vals[@]}"; do
	echo "Delay Primary value: $delay_val"
	for policy_number in 20 40 60 80 99; do
		run_experiment "delay_primary" "$delay_val" "$policy_number" &
	done
done

# Bandwidth primary experiments
echo "Delay Alternate experiments"
delay_vals=("0ms" "10ms" "20ms" "30ms" "40ms" "60ms" "80ms" "100ms")
for delay_val in "${delay_vals[@]}"; do
	echo "Delay Alternate value: $delay_val"
	for policy_number in 20 40 60 80 99; do
		run_experiment "delay_alternate" "$delay_val" "$policy_number" &
	done
done

# primary experiments
echo "Bandwidth Primary experiments"
bandwidth_vals=("75Kbps" "100Kbps" "125Kbps" "150Kbps" "175Kbps" "200Kbps" "225Kbps" "250Kbps" "275Kbps" "300Kbps")
for bandwidth_val in "${bandwidth_vals[@]}"; do
	echo "Bandwidth Primary value: $bandwidth_vals"
	for policy_number in 20 40 60 80 99; do
		run_experiment "bandwidth_primary" "$bandwidth_val" "$policy_number" &
	done
done

wait
echo "NS3 script finished"