#!/bin/bash

echo "cp ../../final/txty.cpp main.cpp"
cp ../../final/txty.cpp main.cpp

echo "sudo make clean"
sudo make clean

echo "sudo make -j 4"
sudo make -j 4


