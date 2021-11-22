#!/bin/bash

SAVEFILE=./txtySave.txt

if [ ! -e "$SAVEFILE" ]; then
  touch txtySave.txt
  echo "0 0 42775 0 16 1 100" > txtySave.txt
fi

sudo ./txty

# Remove comment below when done!
#sudo shutdown now
