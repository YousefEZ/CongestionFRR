#!/bin/bash

# Check if arguments are provided
if [ "$#" -ne 9 ]; then
	echo "Usage: $0 <test_number> <bandwidth_bottleneck> <bandwidth_access> <bandwidth_udp_access> <delay_bottleneck> <delay_access> <delay_alternate> <bandwidth_alternate> <policy>"
	exit 1
fi

# Extract arguments
test_number=$1
bandwidth_bottleneck=$2
bandwidth_access=$3
bandwidth_udp_access=$4
delay_bottleneck=$5
delay_access=$6
delay_alternate=$7
bandwidth_alternate=$8
policy=$9

edit_cpp_files() {
	local filename=$1
	local policy=$2

	awk -v BB="$bandwidth_bottleneck" -v BA="$bandwidth_access" -v BUA="$bandwidth_udp_access" -v DB="$delay_bottleneck" -v DA="$delay_access" -v DALT="$delay_alternate" -v BALT="$bandwidth_alternate" -v POL="$policy" '
        { 
            gsub(/std::string bandwidth_bottleneck = ".+";/, "std::string bandwidth_bottleneck = \"" BB "\";");
            gsub(/std::string bandwidth_access = ".+";/, "std::string bandwidth_access = \"" BA "\";");
            gsub(/std::string bandwidth_udp_access = ".+";/, "std::string bandwidth_udp_access = \"" BUA "\";");
            gsub(/std::string delay_bottleneck = ".+";/, "std::string delay_bottleneck = \"" DB "\";");
            gsub(/std::string delay_access = ".+";/, "std::string delay_access = \"" DA "\";");
            gsub(/std::string delay_alternate = ".+";/, "std::string delay_alternate = \"" DALT "\";");
            gsub(/std::string bandwidth_alternate = ".+";/, "std::string bandwidth_alternate = \"" BALT "\";");
            gsub(/CongestionPolicy = BasicCongestionPolicy<[0-9]+>;$/, "CongestionPolicy = BasicCongestionPolicy<" POL ">;");
            print
        }
    ' "$filename" >"$filename.tmp" && mv "$filename.tmp" "$filename"
}

# Define function to run Docker command for each file
run_docker_command() {
	local filename=$1

	docker-compose run ns3 $filename
}

# Define function to copy files to experiments directory
copy_files_to_experiments() {
	local test_type=$1

	dest_dir="experiments/$test_number/$test_type"

	# Create directory if it doesn't exist
	mkdir -p $dest_dir

	cp -r traces/* $dest_dir
}

# Main script starts here

# Define test types
test_types=("baseline-no-udp" "baseline-udp" "frr" "frr-no-udp")

# Loop through each test type
for test_type in "${test_types[@]}"; do
	# Loop through each C++ file
	for file in combined-$test_type.cc; do
		(
			# Edit lines in the C++ file
			edit_cpp_files "src/$file" "$policy"

			sleep 1

			# Run Docker command
			run_docker_command $file

			# Copy resulting files to experiments directory
			copy_files_to_experiments $test_type
		) &
	done
done

# Wait for all background processes to finish
wait

# Write parameters to a markdown file
echo "### Experiment Parameters" >"experiments/$test_number/parameters.md"
echo "- Bandwidth Bottleneck: $bandwidth_bottleneck" >>"experiments/$test_number/parameters.md"
echo "- Bandwidth Access: $bandwidth_access" >>"experiments/$test_number/parameters.md"
echo "- Bandwidth UDP Access: $bandwidth_udp_access" >>"experiments/$test_number/parameters.md"
echo "- Delay Bottleneck: $delay_bottleneck" >>"experiments/$test_number/parameters.md"
echo "- Delay Access: $delay_access" >>"experiments/$test_number/parameters.md"
echo "- Delay Alternate: $delay_alternate" >>"experiments/$test_number/parameters.md"
echo "- Bandwidth Alternate: $bandwidth_alternate" >>"experiments/$test_number/parameters.md"
echo "- Congestion Policy: $policy" >>"experiments/$test_number/parameters.md"
