#!/bin/bash

SAVEFILE=/home/pi/TxTy/TxTy/final/txtySave.txt

# Load some settings if there aren't any.
if [ ! -e "$SAVEFILE" ]; then
  touch /home/pi/TxTy/TxTy/final/txtySave.txt
  echo "0 0 42775 0 16 1 512" > /home/pi/TxTy/TxTy/final/txtySave.txt
fi

sudo /home/pi/TxTy/TxTy/final/txty

# Remove comment below when done!
#sudo shutdown now
