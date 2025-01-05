#!/bin/bash

set -e

export PATH=$PATH:/home/os2024/os24team27/NachOS-4.0_MP4/code/build.linux


echo "Step 0: Format the disk"
nachos -f

echo "Step 1: Small file"
nachos -cp num_100.txt /small

echo "Step 2: Medium file"
nachos -cp num_1000.txt /medium

echo "Step 3: Large file"
nachos -cp os.pdf /large

echo "Step 4: List their difference"
nachos -D