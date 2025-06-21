#!/usr/bin/env bash

set -e

echo "Cleaning and building..."
make clean
make all

echo -e "\nRunning tests:\n"
mkdir -p output

for f in tests/test*; do
  testname=$(basename "$f")
  echo "Running $testname..."
  "./$f" > "output/${testname}.out"
done

echo -e "\nAll tests completed. Output is in ./output/"
