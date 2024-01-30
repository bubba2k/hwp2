sudo chmod a+rw /dev/ttyUSB0
sudo chmod a+rw /dev/ttyUSB1
minicom -D /dev/ttyUSB0 -b 9600
minicom -D /dev/ttyUSB1 -b 9600
