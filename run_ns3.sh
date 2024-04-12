#!/bin/bash

echo "Running ns3 script"

seeds=("47828392" "1938290" "1940652")

run_experiment() {
	local test_variable=$1
	local test_value=$2
	local policy_threshold=$3
	local dir="traces/$test_variable/$test_value/$policy_threshold"
	for seed in "${seeds[@]}"; do
		mkdir -p "$dir/frr/$seed/1"
		mkdir -p "$dir/frr/$seed/3"
		mkdir -p "$dir/frr-no-udp/$seed/1"
		mkdir -p "$dir/frr-no-udp/$seed/3"
		mkdir -p "$dir/baseline-udp/$seed/1"
		mkdir -p "$dir/baseline-udp/$seed/3"
		mkdir -p "$dir/baseline-no-udp/$seed/1"
		mkdir -p "$dir/baseline-no-udp/$seed/3"

		NS_LOG="" ./ns3 run "scratch/combined-frr.cc --$test_variable=$test_value --tcp_senders=1 --policy_threshold=$policy_threshold --dir=$dir/frr/$seed/1/ --seed=$seed"
		NS_LOG="" ./ns3 run "scratch/combined-frr.cc --$test_variable=$test_value --tcp_senders=3 --policy_threshold=$policy_threshold --dir=$dir/frr/$seed/3/ --seed=$seed"

		NS_LOG="" ./ns3 run "scratch/combined-frr-no-udp.cc --$test_variable=$test_value --tcp_senders=1 --policy_threshold=$policy_threshold --dir=$dir/frr-no-udp/$seed/1/ --seed=$seed"
		NS_LOG="" ./ns3 run "scratch/combined-frr-no-udp.cc --$test_variable=$test_value --tcp_senders=3 --policy_threshold=$policy_threshold --dir=$dir/frr-no-udp/$seed/3/ --seed=$seed"

		NS_LOG="" ./ns3 run "scratch/combined-baseline-udp.cc --$test_variable=$test_value --tcp_senders=1 --policy_threshold=$policy_threshold --dir=$dir/baseline-udp/$seed/1/ --seed=$seed"
		NS_LOG="" ./ns3 run "scratch/combined-baseline-udp.cc --$test_variable=$test_value --tcp_senders=3 --policy_threshold=$policy_threshold --dir=$dir/baseline-udp/$seed/3/ --seed=$seed"

		NS_LOG="" ./ns3 run "scratch/combined-baseline-no-udp.cc --$test_variable=$test_value --tcp_senders=1 --policy_threshold=$policy_threshold --dir=$dir/baseline-no-udp/$seed/1/ --seed=$seed"
		NS_LOG="" ./ns3 run "scratch/combined-baseline-no-udp.cc --$test_variable=$test_value --tcp_senders=3 --policy_threshold=$policy_threshold --dir=$dir/baseline-no-udp/$seed/3/ --seed=$seed"
	done
}

# Delay primary experiments
echo "Delay Primary experiments"
delay_vals=("0ms" "10ms" "20ms" "30ms" "40ms" "50ms" "60ms" "70ms")
for delay_val in "${delay_vals[@]}"; do
	echo "Delay Primary value: $delay_val"
	for policy_number in 20 40 60 80; do
		run_experiment "delay_primary" "$delay_val" "$policy_number" &
	done
done

wait

# Delay alternate experiments
echo "Delay Alternate experiments"
delay_vals=("0ms" "10ms" "20ms" "30ms" "40ms" "60ms" "70ms" "80ms")
for delay_val in "${delay_vals[@]}"; do
	echo "Delay Alternate value: $delay_val"
	for policy_number in 20 40 60 80; do
		run_experiment "delay_alternate" "$delay_val" "$policy_number" &
	done
done

wait

# Bandwidth primary experiments
echo "Bandwidth Primary experiments"
#bandwidth_vals=("200Kbps" "225Kbps" "250Kbps" "275Kbps" "300Kbps" "325Kbps" "350Kbps" "375Kbps" "400Kbps")
# bandwidth_vals=("1.5Mbps" "1.6Mbps" "1.7Mbps" "1.8Mbps" "1.9Mbps" "2.0Mbps" "2.1Mbps" "2.2Mbps" "2.3Mbps" "2.4Mbps" "2.5Mbps")
bandwidth_vals=("1.0Mbps" "1.2Mbps" "1.4Mbps" "1.6Mbps" "1.8Mbps" "2.0Mbps" "2.2Mbps" "2.4Mbps" "2.6Mbps" "2.8Mbps" "3.0Mbps" "3.2Mbps" "3.4Mbps" "3.6Mbps" "3.8Mbps" "4.0Mbps" "4.2Mbps" "4.4Mbps" "4.6Mbps" "4.8Mbps" "5.0Mbps")
# bandwidth_vals=("1.5Mbps" "1.6Mbps" "1.7Mbps" "1.8Mbps" "1.9Mbps" "2.0Mbps" "2.1Mbps" "2.2Mbps" "2.3Mbps" "2.4Mbps" "2.5Mbps" "2.6Mbps" "2.7Mbps" "2.8Mbps" "2.9Mbps" "3.0Mbps" "3.1Mbps" "3.2Mbps" "3.3Mbps" "3.4Mbps" "3.5Mbps" "3.6Mbps" "3.7Mbps" "3.8Mbps" "3.9Mbps" "4.0Mbps" "4.1Mbps" "4.2Mbps" "4.3Mbps" "4.4Mbps" "4.5Mbps" "4.6Mbps" "4.7Mbps" "4.8Mbps" "4.9Mbps" "5.0Mbps")
for bandwidth_val in "${bandwidth_vals[@]}"; do
	echo "Bandwidth Primary value: $bandwidth_vals"
	for policy_number in 20 40 60 80; do
		false && run_experiment "bandwidth_primary" "$bandwidth_val" "$policy_number" &
	done
done

wait

# Bandwidth alternate experiments
echo "Bandwidth alternate experiments"
bandwidth_vals=("1.0Mbps" "1.2Mbps" "1.4Mbps" "1.6Mbps" "1.8Mbps" "2.0Mbps" "2.2Mbps" "2.4Mbps" "2.6Mbps" "2.8Mbps" "3.0Mbps" "3.2Mbps" "3.4Mbps" "3.6Mbps" "3.8Mbps" "4.0Mbps" "4.2Mbps" "4.4Mbps" "4.6Mbps" "4.8Mbps" "5.0Mbps")
# bandwidth_vals=("1.5Mbps" "1.6Mbps" "1.7Mbps" "1.8Mbps" "1.9Mbps" "2.0Mbps" "2.1Mbps" "2.2Mbps" "2.3Mbps" "2.4Mbps" "2.5Mbps" "2.6Mbps" "2.7Mbps" "2.8Mbps" "2.9Mbps" "3.0Mbps" "3.1Mbps" "3.2Mbps" "3.3Mbps" "3.4Mbps" "3.5Mbps" "3.6Mbps" "3.7Mbps" "3.8Mbps" "3.9Mbps" "4.0Mbps" "4.1Mbps" "4.2Mbps" "4.3Mbps" "4.4Mbps" "4.5Mbps" "4.6Mbps" "4.7Mbps" "4.8Mbps" "4.9Mbps" "5.0Mbps")
for bandwidth_val in "${bandwidth_vals[@]}"; do
	echo "Bandwidth Alternate value: $bandwidth_vals"
	for policy_number in 20 40 60 80; do
		false && run_experiment "bandwidth_alternate" "$bandwidth_val" "$policy_number" &
	done
done

wait
echo "NS3 script finished"
