#ifndef _DCIHID_H_
#define _DCIHID_H_

#include <sys/types.h>

// Uncoment if you need debug messages printed:
//#define _DCIHID_DEBUG_

/*
 * device type definitions
 */
#define USB_16PIO   0x01 // USB 16 Channel Photo Input / 16 Channel Photo Output Board
#define USB_LABKIT  0x02 // USB LABKIT
#define USB_16PR    0x03 // USB 16 Channel Photo Input / 16 Channel Relay Output Board
#define USB_STARTER 0x04 // USB STARTER
#define USB_8PR     0x06 // USB 8 Channel Photo Input / 8 Channel Relay Output Board
#define USB_4PR     0x07 // USB 4 Channel Photo Input / 4 Channel Relay Output Board
#define USB_8PI     0x08 // USB 8 Channel Photo Input Board
#define USB_8RO     0x09 // USB 8 Channel Relay Output Board
#define USB_16PI    0x0A // USB 16 Channel Photo Input Board
#define USB_16RO    0x0B // USB 16 Channel Relay Output Board
#define USB_32PI    0x0C // USB 32 Channel Photo Input Board
#define USB_32RO    0x0D // USB 32 Channel Relay Output Board
#define USB_IND     0x0E // USB Industry Board
#define USB_M_4IO   0x10 // USB Mini 4 I/O

#ifdef __cplusplus
extern "C" {
#endif
/*
 * function prototypes
 */
u_int64_t   dcihid_open(const char *dev_name, const u_int card_type, const u_int card_id);
int32_t     dcihid_close(const u_int64_t dcihid_handle);
int32_t     dcihid_write(const u_int64_t dcihid_handle, const u_int32_t addr, const u_int8_t data);
int32_t     dcihid_read(const u_int64_t dcihid_handle, const u_int32_t addr, u_int8_t *data);
u_int       dcihid_assert_card_type(const u_int card_type);
u_int       dcihid_assert_card_id(const u_int card_id);

#ifdef _cplusplus
}
#endif

#endif
