/*
 * File:
 *      main.c
 *
 * Description:
 *      Application to control DCI USB HID devices from
 *      Decision-Computer.
 *
 * History:
 *      2019/08/27: Vasco Baptista: Initial version
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>

#include "dcihid.h"

#define READ            0
#define WRITE_BYTE      1
#define WRITE_SET_BIT   2
#define WRITE_CLEAR_BIT 3
#define UNDEFINED 0xFF

/*
 * Main
 */
int
main(int argc, char *argv[])
{
    char linux_hiddev[64] = "";
    u_int8_t dcihid_card_type = UNDEFINED;
    u_int8_t dcihid_card_id = UNDEFINED;
    u_int8_t port_address = UNDEFINED;
    u_int8_t access_mode = READ;
    u_int8_t data = 0x00;
    
    int opt;
    
    // Exit if the user did not type any arguments:
    if (argc == 1) {
        fprintf(stderr, "No arguments specified. Try '%s -h' for more information.\n", argv[0]);
        return 1; 
    }
    
    // Loop through all arguments:
    while ((opt = getopt(argc, argv, "d:t:i:r:w:f:b:s:c:hv")) != -1) {  
        switch (opt) {
            case 'd':
                // Set the device to use:
                strcpy(linux_hiddev, optarg);
                break;
            case 't':
                // Set the device type:
                dcihid_card_type = (u_int8_t)strtol(optarg, NULL, 0);
                if (!dcihid_assert_card_type(dcihid_card_type)) {
                    fprintf(stderr, "Unknown device type. Try '%s -h' for more information.\n", argv[0]);
                    return 1;
                }
                break;
            case 'i':
                // Set the device ID:
                dcihid_card_id = (u_int8_t)strtol(optarg, NULL, 0);
                if (!dcihid_assert_card_id(dcihid_card_id)) {
                    fprintf(stderr, "Unknown device ID. Try '%s -h' for more information.\n", argv[0]);
                    return 1;
                }
                break;
            case 'r':
                // Set the address to read from:
                access_mode = READ;
                port_address = (u_int8_t)strtol(optarg, NULL, 0);
                break; 
            case 'w':
                // Set the address to write to:
                access_mode = WRITE_BYTE;
                port_address = (u_int8_t)strtol(optarg, NULL, 0);
                break;
            case 'b':
                // Byte to be writen:
                access_mode = WRITE_BYTE;
                data = (u_int8_t)strtol(optarg, NULL, 0);
                break; 
            case 's':
                // Bit to be set later using OR:
                access_mode = WRITE_SET_BIT;
                data = (u_int8_t)atoi(optarg);
                break; 
            case 'c':
                // Bit to be cleared later using AND:
                access_mode = WRITE_CLEAR_BIT;
                data = (u_int8_t)atoi(optarg);
                break; 
            case 'h':
                // Print help:
                printf("%s: Application to control DCI USB HID devices from Decision-Computer\n", argv[0]);
                printf("Usage: %s -d <device> -t <type> -i <id> -r/w <port> [-b <byte> / -s/c <bit>]\n", argv[0]);
                printf("Where:\n");
                printf("  <device> is the linux HID device to use, for example: /dev/usb/hiddev0\n");
                printf("  <type> is the card  type as 0x??:\n");
                printf("      TYPE: DESCRIPTION\n");
                printf("      0x01: USB 16 Channel Photo Input / 16 Channel Photo Output\n");
                printf("      0x02: USB LABKIT\n");
                printf("      0x03: USB 16 Channel Photo Input / 16 Channel Relay Output\n");
                printf("      0x04: USB STARTER\n");
                printf("      0x06: USB 8 Channel Photo Input / 8 Channel Relay Output\n");
                printf("      0x07: USB 4 Channel Photo Input / 4 Channel Relay Output\n");
                printf("      0x08: USB 8 Channel Photo Input\n");
                printf("      0x09: USB 8 Channel Relay Output\n");
                printf("      0x0A: USB 16 Channel Photo Input\n");
                printf("      0x0B: USB 16 Channel Relay Output\n");
                printf("      0x0C: USB 32 Channel Photo Input\n");
                printf("      0x0D: USB 32 Channel Relay Output\n");
                printf("      0x0E: USB Industry\n");
                printf("      0x10: USB Mini 4 I/O\n");
                printf("  <id> is the card ID set on the DIP switch from 0 to 14:\n");
                printf("      ID:  1   2   3   4 \n");
                printf("      0:  OFF OFF OFF OFF\n");
                printf("      1:  ON  OFF OFF OFF\n");
                printf("      2:  OFF ON  OFF OFF\n");
                printf("      3:  ON  ON  OFF OFF\n");
                printf("      4:  OFF OFF ON  OFF\n");
                printf("      5:  ON  OFF ON  OFF\n");
                printf("      6:  OFF ON  ON  OFF\n");
                printf("      7:  ON  ON  ON  OFF\n");
                printf("      8:  OFF OFF OFF ON \n");
                printf("      9:  ON  OFF OFF ON \n");
                printf("      10: OFF ON  OFF ON \n");
                printf("      11: ON  ON  OFF ON \n");
                printf("      12: OFF OFF ON  ON \n");
                printf("      13: ON  OFF ON  ON \n");
                printf("      14: OFF ON  ON  ON \n");
                printf("  -w <port> is the port address to write to, depending on the card type:\n");
                printf("      TYPE PORT: DESCRIPTION\n");
                printf("      0x01 0x02: OUT07 to OUT00\n");
                printf("           0x03: OUT15 to OUT08\n");
                printf("      0x02 0x03: P1D07 to P1D00\n");
                printf("      0x04 0x03: P1D07 to P1D00\n");
                printf("      0x03 0x02: OUT07 to OUT00\n");
                printf("           0x03: OUT15 to OUT08\n");
                printf("      0x06 0x01: OUT07 to OUT00\n");
                printf("           0x02: DIO7 to DIO0\n");
                printf("           0x03: DIO15 to DIO8\n");
                printf("      0x07 0x02: OUT03 to OUT00\n");
                printf("      0x09 0x02: OUT07 to OUT00\n");
                printf("      0x0B 0x02: OUT07 to OUT00\n");
                printf("           0x03: OUT15 to OUT08\n");
                printf("      0x0D 0x00: OUT07 to OUT00\n");
                printf("           0x01: OUT15 to OUT08\n");
                printf("           0x02: OUT23 to OUT16\n");
                printf("           0x03: OUT31 to OUT24\n");
                printf("      0x0E 0x00: Port 0\n");
                printf("           0x01: Port 1\n");
                printf("           0x02: Port 2\n");
                printf("           0x03: Port 3\n");
                printf("           0x04: Port 4\n");
                printf("           0x05: Port 5\n");
                printf("           0x06: Port 6\n");
                printf("           0x07: Port 7\n");
                printf("           0x08: DIO\n");
                printf("           0x0D: IOCONFIG\n");
                printf("      0x10 0x02: OUT03 to OUT00\n");
                printf("  -r <port> is the port address to read from, depending on the card type:\n");
                printf("      TYPE PORT: DESCRIPTION\n");
                printf("      0x01 0x00: IN07 to IN00\n");
                printf("           0x01: IN15 to IN08\n");
                printf("      0x02 0x02: P0D07 to P0D00\n");
                printf("      0x04 0x02: P0D07 to P0D00\n");
                printf("      0x03 0x00: IN07 to IN00\n");
                printf("           0x01: IN15 to IN08\n");
                printf("      0x06 0x00: IN07 to IN00\n");
                printf("           0x02: DIO7 to DIO0\n");
                printf("           0x03: DIO15 to DIO8\n");
                printf("           0x10: JP9/JP10 Settings\n");
                printf("      0x07 0x00: IN03 to IN00\n");
                printf("      0x08 0x00: IN07 to IN00\n");
                printf("      0x0A 0x00: IN07 to IN00\n");
                printf("           0x01: IN15 to IN08\n");
                printf("      0x0C 0x00: IN07 to IN00\n");
                printf("           0x01: IN15 to IN08\n");
                printf("           0x02: IN23 to IN16\n");
                printf("           0x03: IN31 to IN24\n");
                printf("      0x0E 0x00: Port 0\n");
                printf("           0x01: Port 1\n");
                printf("           0x02: Port 2\n");
                printf("           0x03: Port 3\n");
                printf("           0x04: Port 4\n");
                printf("           0x05: Port 5\n");
                printf("           0x06: Port 6\n");
                printf("           0x07: Port 7\n");
                printf("           0x08: DIO\n");
                printf("           0x0D: IOCONFIG\n");
                printf("           0x10: Port 0 default value\n");
                printf("           0x11: Port 1 default value\n");
                printf("           0x12: Port 2 default value\n");
                printf("           0x13: Port 3 default value\n");
                printf("           0x14: Port 4 default value\n");
                printf("           0x15: Port 5 default value\n");
                printf("           0x16: Port 6 default value\n");
                printf("           0x17: Port 7 default value\n");
                printf("           0x18: Port DIO default value\n");
                printf("           0x19: Input/output default setting\n");
                printf("      0x10 0x00: IN03 to IN00\n");
                printf("  -b <byte> is the byte to be written while using -w <port>: 0x00 to 0xFF\n");
                printf("  -s/c <bit> is the bit to be set/clear while using -w <port>: 0 to 7\n");
                printf("Examples:\n");
                printf("  %s -d /dev/usb/hiddev0 -t 0x06 -i 0 -r 0x02\n", argv[0]);
                printf("  %s -d /dev/usb/hiddev0 -t 0x06 -i 0 -w 0x02 -b 0x88\n", argv[0]);
                printf("  %s -d /dev/usb/hiddev0 -t 0x06 -i 0 -w 0x02 -s 5\n", argv[0]);
                printf("  %s -d /dev/usb/hiddev0 -t 0x06 -i 0 -w 0x02 -c 2\n", argv[0]);
                return 0;
                break;
            case 'v':
                printf("%s: Application to control DCI USB HID devices from Decision-Computer\n", argv[0]);
                printf("Version: 1.0 (2019/08/28) coded by Vasco Baptista\n");
                return 0;
                break;
            case ':':
                fprintf(stderr, "Option provided needs a value\n");
                return 1;
                break;
            case '?':
                fprintf(stderr, "Unknown option: %c. Try '%s -h' for more information.\n", optopt, argv[0]);
                return 1;
                break;
        }
    }
    
    // Verify if we have all needed values:
    if (linux_hiddev[0] == '\0') {
        fprintf(stderr, "No Linux HID device specified. Try '%s -h' for more information.\n", argv[0]);
        return 1;
    }
    if (dcihid_card_type == UNDEFINED) {
        fprintf(stderr, "No card type specified. Try '%s -h' for more information.\n", argv[0]);
        return 1;
    }
    if (dcihid_card_id == UNDEFINED) {
        fprintf(stderr, "No card ID specified. Try '%s -h' for more information.\n", argv[0]);
        return 1;
    }
    if (port_address == UNDEFINED) {
        fprintf(stderr, "No port address specified. Try '%s -h' for more information.\n", argv[0]);
        return 1;
    }
    
    printf("Using HID device %s to connect with card of type 0x%02X and ID %u\n", linux_hiddev, dcihid_card_type, dcihid_card_id);
    
    // Open handle:
    u_int64_t dcihid_handle = 0;
    dcihid_handle = dcihid_open(linux_hiddev, dcihid_card_type, dcihid_card_id);
    if (dcihid_handle == 0) {
        fprintf(stderr, "There is no Decision-Computer DCI HID USB device (CardID, CardNO) = (%d, %d) plugged\n", dcihid_card_type, dcihid_card_id);
        return 1;
    }

    // Perform actions:
    u_int8_t tmp;
    switch (access_mode) {
        case READ:
            printf("Reading value on port address: 0x%02X\n", port_address);
            dcihid_read(dcihid_handle, port_address, &data);
            printf("[0x%02X]=0x%02X\n", port_address, data);
            break;
        case WRITE_BYTE:
            printf("Writing value 0x%02X on port address: 0x%02X\n", data, port_address);
            dcihid_write(dcihid_handle, port_address, data);
            printf("[0x%02X]=0x%02X\n", port_address, data);
            break;
        case WRITE_SET_BIT:
            printf("Setting bit %u on port address: 0x%02X\n", data, port_address);
            dcihid_read(dcihid_handle, port_address, &tmp);
            data = tmp | (1 << data);
            dcihid_write(dcihid_handle, port_address, data);
            printf("[0x%02X]=0x%02X\n", port_address, data);
            break;
        case WRITE_CLEAR_BIT:
            printf("Clearing bit %u on port address: 0x%02X\n", data, port_address);
            dcihid_read(dcihid_handle, port_address, &tmp);
            data = tmp & ~(1 << data);
            dcihid_write(dcihid_handle, port_address, data);
            printf("[0x%02X]=0x%02X\n", port_address, data);
            break;
        default:
            fprintf(stderr, "Unexpected access mode\n");
            break;
    }
    
    // Close handle and exit:
    dcihid_close(dcihid_handle);
    return 0;
}
