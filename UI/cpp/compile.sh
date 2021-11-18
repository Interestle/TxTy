#!/bin/bash

mainExe=./main
mainCpp=./main.cpp

if [ -e "$mainExe" ]; then
  echo "cp main main_old/main_old"
  cp main main_old/main_old
fi

if [ -e "$mainCpp" ]; then
  echo "cp main.cpp main_old/main_old.cpp"
  cp main.cpp main_old/main_old.cpp
fi

echo "cp ../../final/txty.cpp main.cpp"
cp ../../final/txty.cpp main.cpp

echo "sudo make clean"
sudo make clean

echo "sudo make -j 4"
sudo make -j 4

if [ -e "./main" ]; then
  echo "cp main ../../final/txty"
  cp main ../../final/txty
fi

