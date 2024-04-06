#!/bin/bash

echo "Running ns3 script"

seeds=("47828392" "1938290" "1940652")

run_experiment() {
	local test_variable=$1
	local test_value=$2
	local policy_threshold=$3
	local dir="traces/$test_variable/$test_value/$policy_threshold"
	for seed in "${seeds[@]}"; do
		mkdir -p "$dir/frr/$seed"
		mkdir -p "$dir/frr-no-udp/$seed"
		mkdir -p "$dir/baseline-udp/$seed"
		mkdir -p "$dir/baseline-no-udp/$seed"

		NS_LOG="" ./ns3 run "scratch/combined-frr.cc --$test_variable=$test_value --tcp_senders=1 --policy_threshold=$policy_threshold --dir=$dir/frr/$seed/ --seed=$seed"
		NS_LOG="" ./ns3 run "scratch/combined-frr-no-udp.cc --$test_variable=$test_value --tcp_senders=1 --policy_threshold=$policy_threshold --dir=$dir/frr-no-udp/$seed/ --seed=$seed"
		NS_LOG="" ./ns3 run "scratch/combined-baseline-udp.cc --$test_variable=$test_value --tcp_senders=1 --policy_threshold=$policy_threshold --dir=$dir/baseline-udp/$seed/ --seed=$seed"
		NS_LOG="" ./ns3 run "scratch/combined-baseline-no-udp.cc --$test_variable=$test_value --tcp_senders=1 --policy_threshold=$policy_threshold --dir=$dir/baseline-no-udp/$seed/ --seed=$seed"
	done
}

# Delay primary experiments
echo "Delay Primary experiments"
delay_vals=("0ms" "10ms" "20ms" "30ms" "40ms" "60ms" "80ms" "100ms")
for delay_val in "${delay_vals[@]}"; do
	echo "Delay Primary value: $delay_val"
	for policy_number in 20 40 60 80 99; do
		run_experiment "delay_primary" "$delay_val" "$policy_number" &
	done
done

# Delay alternate experiments
echo "Delay Alternate experiments"
delay_vals=("0ms" "10ms" "20ms" "30ms" "40ms" "60ms" "80ms" "100ms")
for delay_val in "${delay_vals[@]}"; do
	echo "Delay Alternate value: $delay_val"
	for policy_number in 20 40 60 80 99; do
		run_experiment "delay_alternate" "$delay_val" "$policy_number" &
	done
done

# Bandwidth primary experiments
echo "Bandwidth Primary experiments"
bandwidth_vals=("75Kbps" "100Kbps" "125Kbps" "150Kbps" "175Kbps" "200Kbps" "225Kbps" "250Kbps" "275Kbps" "300Kbps")
for bandwidth_val in "${bandwidth_vals[@]}"; do
	echo "Bandwidth Primary value: $bandwidth_vals"
	for policy_number in 20 40 60 80 99; do
		run_experiment "bandwidth_primary" "$bandwidth_val" "$policy_number" &
	done
done

# Bandwidth alternate experiments
echo "Bandwidth alternate experiments"
bandwidth_vals=("75Kbps" "100Kbps" "125Kbps" "150Kbps" "175Kbps" "200Kbps" "225Kbps" "250Kbps" "275Kbps" "300Kbps")
for bandwidth_val in "${bandwidth_vals[@]}"; do
	echo "Bandwidth Alternate value: $bandwidth_vals"
	for policy_number in 20 40 60 80 99; do
		run_experiment "bandwidth_alternate" "$bandwidth_val" "$policy_number" &
	done
done

wait
echo "NS3 script finished"
