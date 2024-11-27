#!/bin/sh
../build.linux/nachos -f
../build.linux/nachos -cp hw4t1 /hw4t1
../build.linux/nachos -e /hw4t1
../build.linux/nachos -p /file1
../build.linux/nachos -cp hw4t2 /hw4t2
../build.linux/nachos -e /hw4t2
