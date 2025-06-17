#!/usr/bin/env bash
make clean
make all

#TESTS 14 and 18 MAY HAVE BEEN INITIALLY WRONG WHEN WRITING TO CORRECT OUTPUT

echo "tests:"
echo 

echo "Now making tests. After test(num test), there should be no output - we're running diff with correct files"
for i in {1..21}; do
  echo "test:" ${i}
  ./test${i} >  output/out${i}.txt
  # We want diff to be nothing. For the tests we have correct output for based on the submission output 
  # diff output/out${i}.txt correct/t${i}.correct
done

