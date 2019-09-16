# exololi

1. Objective: Establish communication (read and write) in a CAN bus in order to control a Maxon EC90 motor via EPOS2 driver using a Raspberry Pi 3B+ and a PiCAN2 hat.

2. The sendump.c was written based in the linux-can-utils examples (http://www.skpang.co.uk/dl/can-test_pi2.zip) and the socketCAN documentation (https://www.kernel.org/doc/Documentation/networking/can.txt). This code implements:

-EPOS2 configuration: Start Up and PDO Config.

-Sends a position set point in qc (quad counts).

-Requests, reads and print in the terminal the current position, velocity and electrical current.

3. Using the PiCAN2 requires this in RPi SO: (more info in: http://skpang.co.uk/catalog/images/raspberrypi/pi_2/PICAN2UG13.pdf)

Add the overlays by: 

sudo nano /boot/config.txt

Add these 3 lines to the end of file: (here it is a little different, note the missing "overlay" in the 3rd line)

dtparam=spi=on

dtoverlay=mcp2515-can0,oscillator=16000000,interrupt=25

dtoverlay=spi-bcm2835

4. It is always necessary bring up the CAN interface once after SO start, then:

sudo /sbin/ip link set can0 up type can bitrate 1000000

(note the bitrate = 1Mb/s)

5. Compiling the code:

gcc -c sendump.c

gcc -o sendump lib.o sendump.o

6. Running the code:

It is possible to pass a set point argument in qc, like this:

./sendump 3200

In this case, 3200 qc represents one turn in the EC90 motor. Without this value the program uses a default value.

7. Checking out the CAN bus:

The candump.c code is useful to check all data exchanged in the CAN bus. It is necessary compile and run:

./candump can0
 
