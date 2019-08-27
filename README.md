# Linux driver and CLI application for Decision-Computer HID USB Digital IO boards
This repository contains the driver source code written in C to control HID USB Digital IO boards from [Decision-Computer](https://www.decision-computer.de/index.html).  
Additionaly it is included the source code to build a standalone application that runs on Linux terminal and allows you to control the boards. You need to have root previleges to run this tool or use this driver on your projects.  

The boards can be found and bought from the producer here: [https://www.decision-computer.de/Produkte/Menue-usb-e.html](https://www.decision-computer.de/Produkte/Menue-usb-e.html)  
The link contains additional **Windows drivers**, all **documentation** and additional **source code**.

Part of the code found on this repository was based on the source code provided by the producer.

## Compatible devices:
The following devices are compatible with this driver and standalone application:
  - USB 16 Channel Photo Input / 16 Channel Photo Output Board
  - USB LABKIT
  - USB 16 Channel Photo Input / 16 Channel Relay Output Board
  - USB STARTER
  - USB 8 Channel Photo Input / 8 Channel Relay Output Board
  - USB 4 Channel Photo Input / 4 Channel Relay Output Board
  - USB 8 Channel Photo Input Board
  - USB 8 Channel Relay Output Board
  - USB 16 Channel Photo Input Board
  - USB 16 Channel Relay Output Board
  - USB 32 Channel Photo Input Board
  - USB 32 Channel Relay Output Board
  - USB Industry Board
  - USB Mini 4 I/O

## Routines for accessing the device:
The routines for accessing the device can be found on these files. You may use these files on your own project so you can communicate with the boards directly. *If you do not want to use them on your source code you can read below and use the standalone terminal application instead.*
```C
dcihid.h
dcihid.c
```

## Source code for the standalone application:
The source code for the standalone application can be found on this file:
```C
main.c
```

## Compiling the source code:
Use make to build the standalone application or modify it for your own liking. Note that you need a 64 bit Linux for the drivers to work, read last topic if you wish to adapt this to run on 32 bit Linux:
```shell
make clean
make
```

## Running:
Just run the application with '-h' from the terminal and you will see the help. Note that **you need root previleges**!
```shell
./DecisionUsb-dio -h
```

## Examples: 
Here are some examples on how to run the standalone application:
```shell
# Read port 0x02 on HID device hiddev0, of type 0x06 and ID 0:
./DecisionUsb-dio -d /dev/usb/hiddev0 -t 0x06 -i 0 -r 0x02
# Write byte 0x88 on port 0x02 on HID device hiddev0, of type 0x06 and ID 0:
./DecisionUsb-dio -d /dev/usb/hiddev0 -t 0x06 -i 0 -w 0x02 -b 0x88
# Set bit 5 on port 0x02 on HID device hiddev0, of type 0x06 and ID 0:
./DecisionUsb-dio -d /dev/usb/hiddev0 -t 0x06 -i 0 -w 0x02 -s 5
# Clear bit 2 on port 0x02 on HID device hiddev0, of type 0x06 and ID 0:
./DecisionUsb-dio -d /dev/usb/hiddev0 -t 0x06 -i 0 -w 0x02 -c 2
```
You will have something similar to this as output:
```shell
$ ./DecisionUsb-dio -d /dev/usb/hiddev1 -t 0x06 -i 0 -w 0x01 -b 0x46
Using HID device /dev/usb/hiddev1 to connect with card of type 0x06 and ID 0
Writing value 0x46 on port address: 0x01
[0x01]=0x46
$ ./DecisionUsb-dio -d /dev/usb/hiddev1 -t 0x06 -i 0 -r 0x01
Using HID device /dev/usb/hiddev1 to connect with card of type 0x06 and ID 0
Reading value on port address: 0x01
[0x01]=0x46
```

## Using in 32 bit Linux:
To run this driver and use it on 32bit Linux you may need to change the variable sizes from `u_int64_t` to `u_int32_t` on the driver .c and .h file
