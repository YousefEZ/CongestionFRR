#!/bin/bash

cp user_src/* scratch/

./ns3 build

if [ $? -ne 0 ]; then
  echo "ns3 build failed"
  exit 1
fi

if [ -z "$1" ]; then

  DIRECTORY="scratch"

  for fullpath in "$DIRECTORY"/*.cc
  do
    filename=$(basename -- "$fullpath")
    name="${filename%.*}"
    ./ns3 run "$name"

    if [ $? -ne 0 ]; then
      echo "multiple ./ns3 run failed at $name"
      exit 1
    fi
  done
  exit 0
fi

./ns3 run "scratch/$1"
if [ $? -ne 0 ]; then
  echo "single ./ns3 run for $name failed"
  exit 1
fi
exit 0
