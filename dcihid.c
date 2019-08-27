/*
 * File:
 *      dcihid.c
 *
 * Description:
 *      The system layer upon the hiddev interface to DCI USB HID devices
 *      from Decision-Computer. Driver adapted to 64bit OS.
 *
 * History:
 *      2009/04/03: David Kao:      Initially created
 *      2019/08/27: Vasco Baptista: Adapted for 64bit OS, visual changes
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/hiddev.h>
#include <errno.h>

#include "dcihid.h"


/*
 * hardware configuration
 */
#define eReport_ID  0

/*
 * HID_REPORT_TYPE_INPUT
 */
enum {
    eCard_ID = 0,
    eCard_Num = 1,
    eport1 = 2,
    eport2 = 3,
    eport3 = 4,
    eport4 = 5,
    eadc_index = 6,
    elower_adc_result = 7,
    eupper_adc_result = 8
};
#define DCIHID_REPORT_INPUT_MAX 9

/*
 * HID_REPORT_TYPE_OUTPUT
 */
enum {
    eDIO_address = 0,
    eDIO_data = 1,
    eChipSel = 2,
    eControl = 3
};
#define DCIHID_REPORT_OUTPUT_MAX    5   

#define DCIHID_16PR_VID     0x10C4
#define DCIHID_16PR_PID     0x81B9


/*
 * type declarations
 */
struct dcihid_dev {
    char                        devname[64];
    char                        prodname[256];
    int                         fd;
    struct hiddev_devinfo       device_info;
    struct hiddev_report_info   report_info_input;
    struct hiddev_field_info    field_info_input;
    unsigned                    usage_code_input;
    struct hiddev_report_info   report_info_output;
    struct hiddev_field_info    field_info_output;    
    unsigned                    usage_code_output;
};
typedef struct dcihid_dev *dcihid_dev_t;
#define DCIHID_DEV_SZ       (sizeof(struct dcihid_dev))
#define DCIHID_DEV_NULL     ((dcihid_dev_t)0)
 

/*
 * functions
 */
 
u_int64_t 
dcihid_open(const char *dev_name, const u_int card_type, const u_int card_id) {
    dcihid_dev_t                dcihid_dev;
    char                        devname[64];  
    char                        prodname[256];    
    int                         fd;
    struct hiddev_devinfo       device_info;
    struct hiddev_report_info   report_info, report_info_input, report_info_output;
    struct hiddev_field_info    field_info, field_info_input, field_info_output;
    struct hiddev_usage_ref     usage_ref;
    unsigned                    usage_code_input, usage_code_output;
    unsigned int                yalv;
    int                         report_type;

    /* ioctl() requires a file descriptor, so we check if we got one, and then open it */
    strcpy(devname, dev_name);

    if ((fd = open(devname, O_RDONLY)) < 0) {
        perror("dcihid open");
        return (u_int64_t)DCIHID_DEV_NULL;
    }

    /* Take out some device information */
    if (-1 == ioctl(fd, HIDIOCGDEVINFO, &device_info)) {
        fprintf(stderr, "ioctl: HIDIOCGDEVINFO: %s\n", strerror(errno));
        return (u_int64_t)DCIHID_DEV_NULL;
    }

#ifdef _DCIHID_DEBUG_
    printf("Device info:\n");
    printf("  bustype=0x%x busnum=0x%x devnum=%u ifnum=%u\n", device_info.bustype, device_info.busnum, device_info.devnum, device_info.ifnum);
    printf("  vendor=0x%04hx product=0x%04hx version=0x%04hx num_applications=%u\n", device_info.vendor, device_info.product, device_info.version, device_info.num_applications);
#endif

    /* Check vendor ID and product ID: */
    if (device_info.vendor != (short)DCIHID_16PR_VID && device_info.product != (short)DCIHID_16PR_PID) {
        return (u_int64_t)DCIHID_DEV_NULL;
    }

    /* Get product name */
    if (-1 == ioctl(fd, HIDIOCGNAME(sizeof(devname)), &prodname)) {
        fprintf(stderr, "ioctl: HIDIOCGNAME: %s\n", strerror(errno));
        return (u_int64_t)DCIHID_DEV_NULL;
    }
    
#ifdef _DCIHID_DEBUG_
    printf("  name= \"%s\"\n\n", prodname);
#endif

    /* Initialise the internal report structures */
    if (-1 == ioctl(fd, HIDIOCINITREPORT)) {
        fprintf(stderr, "HIDIOCINITREPORT: %s\n", strerror(errno));
        return (u_int64_t)DCIHID_DEV_NULL;
    }

    /* Get report info */
    for (report_type = HID_REPORT_TYPE_MIN; report_type <= HID_REPORT_TYPE_MAX; report_type++) {
        report_info.report_type = report_type;
        report_info.report_id = HID_REPORT_ID_FIRST;
            
        while (ioctl(fd, HIDIOCGREPORTINFO, &report_info) >= 0) {
            if (report_info.report_id != eReport_ID) {
                fprintf(stderr, "Invalid report ID, %d, can not be handled\n", report_info.report_id);
                return (u_int64_t)DCIHID_DEV_NULL;
            }
            
            switch (report_info.report_type) {
                case HID_REPORT_TYPE_INPUT:
                case HID_REPORT_TYPE_OUTPUT:
                    break;
                default:
                    fprintf(stderr, "Invalid report type, %d, can not be handled\n", report_info.report_type);  
                    return (u_int64_t)DCIHID_DEV_NULL;
                    break;
            }

#ifdef _DCIHID_DEBUG_
            printf("  Report[ID, Type] [%d, %d] with (%d fields)\n", report_info.report_id, report_info.report_type, report_info.num_fields);
#endif

            if (report_info.num_fields != 1) {
                fprintf(stderr, "Invalid report with %d fields, can not be handled\n", report_info.num_fields);
                return (u_int64_t)DCIHID_DEV_NULL;
            }
                
            memset(&field_info, 0, sizeof(struct hiddev_field_info));
            field_info.report_type = report_info.report_type;
            field_info.report_id = report_info.report_id;
            field_info.field_index = 0;
            ioctl(fd, HIDIOCGFIELDINFO, &field_info);
                
            switch (report_info.report_type) {
                case HID_REPORT_TYPE_INPUT:
                    memcpy(&report_info_input, &report_info, sizeof(struct hiddev_report_info));
                    memcpy(&field_info_input, &field_info, sizeof(struct hiddev_field_info));
                    break;
                case HID_REPORT_TYPE_OUTPUT:
                    memcpy(&report_info_output, &report_info, sizeof(struct hiddev_report_info));
                    memcpy(&field_info_output, &field_info, sizeof(struct hiddev_field_info));
                    break;
            }
            report_info.report_id |= HID_REPORT_ID_NEXT;
        }
    }
         
    /* HID_REPORT_TYPE_INPUT */
    memcpy(&report_info, &report_info_input, sizeof(struct hiddev_report_info));
    if (ioctl(fd, HIDIOCGREPORT, &report_info) == -1) {
        fprintf(stderr, "HIDIOCGREPORT: %s\n", strerror(errno));
        return -1;
    }
    memcpy(&field_info, &field_info_input, sizeof(field_info_input));

#ifdef _DCIHID_DEBUG_
    printf("  Report[ID, Type] [%d, %d] with (%d fields)\n", report_info.report_id, report_info.report_type, report_info.num_fields);
    printf("    Field %d: app: %04x phys %04x flags %x (%d usages) unit %x exp %d\n",
           field_info.field_index, field_info.application, field_info.physical, field_info.flags, field_info.maxusage, field_info.unit, field_info.unit_exponent);
#endif

    /* To get usage code */
    usage_ref.report_type = field_info.report_type;
    usage_ref.report_id = field_info.report_id;
    usage_ref.field_index = 0;
    usage_ref.usage_index = 0;
    if (ioctl(fd, HIDIOCGUCODE, &usage_ref) == -1) {
        fprintf(stderr, "HIDIOCGUCODE: %s\n", strerror(errno));
        return (u_int64_t)DCIHID_DEV_NULL;
    }
    usage_code_input = usage_ref.usage_code;
    
    /* To get usages */
    u_int8_t matched = 1;

    memset(&usage_ref, 0, sizeof(usage_ref));
    for (yalv = 0; yalv < field_info.maxusage; yalv++) {
        usage_ref.report_type = field_info.report_type;
        usage_ref.report_id = field_info.report_id;
        usage_ref.field_index = 0;
        usage_ref.usage_index = yalv;
        if (ioctl(fd, HIDIOCGUSAGE, &usage_ref) == -1) {
            fprintf(stderr, "HIDIOCGUSAGE: %s\n", strerror(errno));
            return (u_int64_t)DCIHID_DEV_NULL;
        }

        if (yalv >= DCIHID_REPORT_INPUT_MAX) continue;

        switch (yalv) {
        case eCard_ID:
            if (usage_ref.value != card_type) matched = 0;
            break;
        case eCard_Num:
            if (usage_ref.value != card_id) matched = 0;
            break;
        }
    }
    if (!matched) return (u_int64_t)DCIHID_DEV_NULL;
    
    /* HID_REPORT_TYPE_OUTPUT */
    memcpy(&report_info, &report_info_output, sizeof(struct hiddev_report_info));
    memcpy(&field_info, &field_info_output, sizeof(struct hiddev_field_info));

#ifdef _DCIHID_DEBUG_
    printf("  Report[ID, Type] [%d, %d] with (%d fields)\n", report_info.report_id, report_info.report_type, report_info.num_fields);
    printf("    Field %d: app: %04x phys %04x flags %x (%d usages) unit %x exp %d\n", 
           field_info.field_index, field_info.application, field_info.physical, field_info.flags, field_info.maxusage, field_info.unit, field_info.unit_exponent);
#endif
    
    /* To get usage code */
    usage_ref.report_type = field_info.report_type;
    usage_ref.report_id = field_info.report_id;
    usage_ref.field_index = 0;
    usage_ref.usage_index = 0;
    if (ioctl(fd, HIDIOCGUCODE, &usage_ref) == -1) {
        fprintf(stderr, "HIDIOCGUCODE: %s\n", strerror(errno));
        return (u_int64_t)DCIHID_DEV_NULL;
    }
    usage_code_output = usage_ref.usage_code;

    /* To get usages */
    memset(&usage_ref, 0, sizeof(usage_ref));
    for (yalv = 0; yalv < field_info.maxusage; yalv++) {
        usage_ref.report_type = field_info.report_type;
        usage_ref.report_id = field_info.report_id;
        usage_ref.field_index = 0;
        usage_ref.usage_index = yalv;
        if (ioctl(fd, HIDIOCGUSAGE, &usage_ref) == -1) {
            fprintf(stderr, "HIDIOCGUSAGE: %s\n", strerror(errno));
            return (u_int64_t)DCIHID_DEV_NULL;
        }
        
        if (yalv >=  DCIHID_REPORT_OUTPUT_MAX) continue;    
    }
    
    dcihid_dev = (dcihid_dev_t)malloc(sizeof(struct dcihid_dev));
    if (dcihid_dev == DCIHID_DEV_NULL) return (u_int64_t)DCIHID_DEV_NULL;
    
    strcpy(dcihid_dev->devname, devname);
    dcihid_dev->fd = fd;
    strcpy(dcihid_dev->prodname, prodname);
    memcpy(&dcihid_dev->device_info, &device_info, sizeof(struct hiddev_devinfo));
    memcpy(&dcihid_dev->report_info_input, &report_info_input, sizeof(struct hiddev_report_info));
    memcpy(&dcihid_dev->field_info_input, &field_info_input, sizeof(struct hiddev_field_info));
    dcihid_dev->usage_code_input = usage_code_input;
    memcpy(&dcihid_dev->report_info_output, &report_info_output, sizeof(struct hiddev_report_info));
    memcpy(&dcihid_dev->field_info_output, &field_info_output, sizeof(struct hiddev_field_info));
    dcihid_dev->usage_code_output = usage_code_output;
    
    return (u_int64_t)dcihid_dev;
}

int32_t
dcihid_close(u_int64_t const dcihid_handle) {
    dcihid_dev_t dcihid_dev = (dcihid_dev_t)dcihid_handle;
    close(dcihid_dev->fd);
    free(dcihid_dev);
    return 0;
}

int32_t
dcihid_write(const u_int64_t dcihid_handle, const u_int32_t addr, const u_int8_t data) {
    dcihid_dev_t                dcihid_dev = (dcihid_dev_t)dcihid_handle;
    int                         fd = dcihid_dev->fd;
    struct hiddev_field_info    *field_info = &dcihid_dev->field_info_output;
    struct hiddev_report_info   report_info;
    struct hiddev_usage_ref     usage_ref;
    unsigned                    usage_code_output = dcihid_dev->usage_code_output;

    /* 
     * To actually send a value to the device, perform a SUSAGE first,
     * followed by a SREPORT. 
     * To do a GUSAGE/SUSAGE, fill in at least usage_code,
     * report_type and report_id.  Set report_id to REPORT_ID_UNKNOWN
     * if the rest of the fields are unknown.  
     * Otherwise use a usage_ref struct filled in from a previous 
     * successful GUSAGE/SUSAGE call to save time. 
     */

    memset(&usage_ref, 0, sizeof(usage_ref));
    usage_ref.usage_index = eDIO_address;
    usage_ref.value       = addr;
    usage_ref.report_type = field_info->report_type;
    usage_ref.report_id   = field_info->report_id;
    usage_ref.field_index = 0;
    usage_ref.usage_code  = usage_code_output;
    if (ioctl(fd, HIDIOCSUSAGE, &usage_ref) == -1) {
        fprintf(stderr, "HIDIOCSUSAGE: %s\n", strerror(errno));
        return -1;
    }

    memset(&usage_ref, 0, sizeof(usage_ref));
    usage_ref.usage_index = eDIO_data;
    usage_ref.value       = (u_int32_t)data;
    usage_ref.report_type = field_info->report_type;
    usage_ref.report_id   = field_info->report_id;
    usage_ref.field_index = 0;
    usage_ref.usage_code  = usage_code_output;
    if (ioctl(fd, HIDIOCSUSAGE, &usage_ref) == -1) {
        fprintf(stderr, "HIDIOCSUSAGE: %s\n", strerror(errno));
        return -1;
    }

    report_info.report_type = HID_REPORT_TYPE_OUTPUT;
    report_info.report_id = 0x00;
    report_info.num_fields = 1;
    if (ioctl(fd, HIDIOCSREPORT, &report_info) == -1) {
        fprintf(stderr, "HIDIOCSREPORT: %s\n", strerror(errno));
        return -1;
    }
 
    return 0;
}

int32_t
dcihid_read(const u_int64_t dcihid_handle, const u_int32_t addr, u_int8_t *data) {
    dcihid_dev_t                dcihid_dev = (dcihid_dev_t)dcihid_handle;
    int                         fd = dcihid_dev->fd;
    struct hiddev_report_info   report_info;
    struct hiddev_field_info    field_info;
    struct hiddev_usage_ref     usage_ref;
    unsigned                    usage_code_input = dcihid_dev->usage_code_input;
    
    /* HID_REPORT_TYPE_INPUT */
    memcpy(&report_info, &dcihid_dev->report_info_input, sizeof(struct hiddev_report_info));
    memcpy(&field_info, &dcihid_dev->field_info_input, sizeof(struct hiddev_field_info));

    /* To get one report */
    if (ioctl(fd, HIDIOCGREPORT, &report_info) == -1) {
        fprintf(stderr, "HIDIOCGREPORT: %s\n", strerror(errno));
        return -1;
    }

    memset(&usage_ref, 0, sizeof(usage_ref));
    usage_ref.report_type = field_info.report_type;
    usage_ref.report_id = field_info.report_id;
    usage_ref.field_index = 0;
    usage_ref.usage_index = eport1 + addr;
    usage_ref.usage_code  = usage_code_input;
    if (ioctl(fd, HIDIOCGUSAGE, &usage_ref) == -1) {
        fprintf(stderr, "HIDIOCGUSAGE: %s\n", strerror(errno));
        return -1;
    }
    *data = ~usage_ref.value & 0xFF;

    return 0;
}

u_int
dcihid_assert_card_type(const u_int card_type) {
    switch (card_type) {
        case USB_16PIO:
        case USB_LABKIT:
        case USB_16PR:
        case USB_STARTER:
        case USB_8PR:
        case USB_4PR:
        case USB_8PI:
        case USB_8RO:
        case USB_16PI:
        case USB_16RO:
        case USB_32PI:
        case USB_32RO:
        case USB_IND:
        case USB_M_4IO:
            return 1;
            break;
    }
    return 0;
}

u_int
dcihid_assert_card_id(const u_int card_id) {
    if (card_id <= 14) return 1;
    return 0;
}
