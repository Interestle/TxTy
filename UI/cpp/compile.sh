#!/bin/bash

mainExe=./main
mainCpp=./main.cpp
saveBackup=./txtySave.txt

if [ -e "$mainExe" ]; then
  echo "cp main main_old/main_old"
  cp main main_old/main_old
fi

if [ -e "$mainCpp" ]; then
  echo "cp main.cpp main_old/main_old.cpp"
  cp main.cpp main_old/main_old.cpp
fi

if [ -e "$saveBackup" ]; then
  echo "cp txtySave.txt main_old/txtySave.txt"
  cp txtySave.txt main_old/txtySave.txt
else
  # No save file. Make one with defaults
  touch txtySave.txt
  echo "0 0 42775 0 16 1 512" > txtySave.txt
fi

# Move current working cpp to here.
echo "cp ../../final/txty.cpp main.cpp"
cp ../../final/txty.cpp main.cpp

echo "sudo make clean"
sudo make clean

echo "sudo make -j 4"
sudo make -j 4

# If successful, the new main file was created, move to final location.
if [ -e "./main" ]; then
  echo "cp main ../../final/txty"
  cp main ../../final/txty
fi

