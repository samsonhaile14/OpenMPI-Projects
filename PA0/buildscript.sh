#!/bin/bash
BUILD_DIR="build/"

# building setup
if [ ! -d "$BUILD_DIR" ]; then
	mkdir build
fi
cd build
cp ../makefile .

# building
make

# running
make run
# be sure to add more commands from your makefile here
# for example, you may create a target called run_single and run_double
# (for running on a double box) that contains some special flags.
# so you may include echo that says what you're doing, and then do it
# Ex:)
# echo "PA1 on single box" && make run_single

# cleaning (optional for build script)
# make clean
