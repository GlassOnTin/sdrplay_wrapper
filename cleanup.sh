#!/bin/bash
# cleanup.sh

# Clean build
rm -rf build
mkdir build

# Clean Python artifacts
rm -rf ../sdrplay/__pycache__
rm -f ../sdrplay/_sdrplay.so
rm -f ../sdrplay/sdrplay.py

# Clean test artifacts
rm -rf ../tests/__pycache__
rm -f ../tests/test_sdrplay_api
