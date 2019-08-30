#!/usr/bin/python
__author__ = "Vasco baptista"
__copyright__ = "Copyright 2019, Vasco Baptista"
__version__ = "1.0"
__maintainer__ = "Vasco Baptista"
__email__ = "vascojdb@gmail.com"

import sys
import os

class DecisionUsbdio:
    """
    Decision-Computer Python wrapper for the Linux CLI application 'DecisionUsbdio'
    Adapted to the 8 Relay + 8 opto + 16 DIO board
    """
    def __init__(self, linuxhiddev='/dev/usb/hiddev1', card_type=0x06, card_id=0, silent=True):
        self.linuxhiddev = linuxhiddev
        self.card_type = card_type
        self.card_id = card_id
        self.silent = silent
        
        # Check if the binary DecisionUsbdio is installed on the system:
        if os.popen("which DecisionUsbdio").read() == "":
            print('The binary is not installed in the system. Please compile and install the binary.')
            exit(1)

    def __send_command(self, port, rw='r', w_action='b', data=0):
        port = int(port, 0)
        data = int(data, 0)
        if rw == 'r':
            answer = os.popen("DecisionUsbdio -d {} -t {} -i {} -r {}".format(self.linuxhiddev, self.card_type, self.card_id, hex(port))).read()
        else:
            answer = os.popen("DecisionUsbdio -d {} -t {} -i {} -w {} -{} {}".format(self.linuxhiddev, self.card_type, self.card_id, hex(port), w_action, data)).read()
        if not self.silent:
            print(answer),
        # Extract and return the value:
        lines = answer.split('\n')
        for line in lines:
            if line.startswith('[0x'):
                addr, val = line.split('=')
                return int(val, 0)

    def read_byte(self, port):
        return self.__send_command(str(port), 'r')

    def write_byte(self, port, byte):
        return self.__send_command(str(port), 'w', 'b', str(byte))

    def set_bit(self, port, bit):
        return self.__send_command(str(port), 'w', 's', str(bit))

    def clear_bit(self, port, bit):
        return self.__send_command(str(port), 'w', 'c', str(bit))


def print_usage():
    """ Prints usage """
    print('{} - A Python wrapper for the DecisionUsbdio binary'.format(sys.argv[0]))
    print('Usage: python {} read|write|set|clear <port> <[byte/bit]>'.format(sys.argv[0]))
    print('Where: <port> is the card port to read from or write to (0x??) - See card manual')
    print('       <byte/bit> is the byte (0-255 or 0x00-0xFF) to write, or bit (0-7) to set/clear')


if __name__ == "__main__":

    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print_usage()
        exit(1)

    # Gets the action and the port to be used, also the value if specified:
    action = sys.argv[1]
    port = sys.argv[2]
    value = sys.argv[3] if len(sys.argv) > 3 else 0

    # Creates the object:
    dev = DecisionUsbdio(linuxhiddev='/dev/usb/hiddev1', card_type=0x06, card_id=0, silent=False)
    
    # Perform action:
    if action == 'read':
        value = dev.read_byte(port)
        print(value)
    elif action == 'write':
        value = dev.write_byte(port, value)
        print(value)
    elif action == 'set':
        value = dev.set_bit(port, value)
        print(value)
    elif action == 'clear':
        value = dev.clear_bit(port, value)
        print(value)
    else:
        print_usage()
        exit(1)
