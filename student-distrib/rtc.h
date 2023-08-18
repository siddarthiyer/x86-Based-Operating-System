/** rtc.h
 * RTC Driver
*/

/**
 * RESOURCES:
 *      protocol: 
 *          https://www.compuphase.com/int70.txt
 *      register values:
 *          https://www.ti.com/lit/ds/symlink/bq3285.pdf
*/

#ifndef _RTC_H
#define _RTC_H
#include "types.h"
#include "tests.h"



/*************************** Constants ***********************************/
/* Ports that the RTC interfaces with */
#define INDEX       0x70
#define DATA        0x71

/* Three available registers in the RTC */

/** Four registers on the RTC
 * A: Used to select an interrupt rate.
 *      bits[6:4]: Base frequency. Default is 32,768 Hz
 *      bits[3:0]: Divider for the base frequency. Resulting frequency will be used for IRQ8.
 *                 Default value is 0110 resulting in 1,024 Hz interrupt rate.
 * 
 * B: flags register
 *      bit 6    : Periodic Interrupt Enable bit (PIE). enables/disables periodic interrupts
 * 
 * C: Information on the type of interrupt that occurred. Must read register C
 *    after each interrupt even if we dont care about value, or IRQ8 will not be generated again
 * 
 * D: dont really care tbh, its the default register tho
 *                
*/
#define REGISTER_A  0x0A 
#define REGISTER_B  0x0B 
#define REGISTER_C  0x0C
#define REGISTER_D  0x0D

#define REGISTER_SECONDS      0x00
#define REGISTER_MINUTES      0x02
#define REGISTER_HOURS        0x04
#define REGISTER_WEEKDAY      0x06
#define REGISTER_DAY_OF_MONTH 0x07
#define REGISTER_MONTH        0x08
#define REGISTER_YEAR         0x09
#define REGISTER_CENTURY      0x32


// Periodic Interrupt Enable bit of register B -> bit 6
#define PIE_ENABLE  0x40

/* NMI Enable/Disable is bit 7 of port 0x70 */
#define DISABLE_NMI 0x80
#define ENABLE_NMI  0x7F

/** based on the conversion:
 *  binary = ((bcd / 16) * 10) + (bcd & 0xf) */
#define BCD_TO_DECIMAL(bcd) ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0xF)
#define PM 0x80


/*************************************************************************/
enum day {
    sunday    = 1,
    monday    = 2,
    tuesday   = 3,
    wednesday = 4,
    thursday  = 5,
    friday    = 6,
    saturday  = 7
};

typedef struct time_t {
    uint8_t seconds;      // 0-59
    uint8_t minutes;      // 0-59
    uint8_t hours;        // 0–23 in 24-hour mode, 1–12 in 12-hour mode, highest bit set if pm
    uint8_t pm;

    uint8_t weekday;      // 1–7, Sunday = 1
    uint8_t day_of_month; // 1–31
    uint8_t month;        // 1-12
    uint8_t year;        // 0-99
    uint8_t century;      // 19-20
} time_t;


/* initialize the RTC */
void rtc_init();

/* handle rtc interrupts */
void rtc_handler(void);

/******* Driver Functions *******/
int rtc_read(int32_t fd, int8_t* buf, int32_t nbytes);
int rtc_write(int32_t fd, const int8_t* buf, int32_t nbytes);
int rtc_open(const int8_t* filename);
int rtc_close(int32_t fd);

void rtc_delay(uint8_t seconds);
void rtc_delay_inv(uint8_t inv_seconds);
struct time_t* rtc_get_time();
uint8_t rtc_get_time_seconds();

#if RTC_WRITE_TEST
/* write the input frequency to the buffer*/
void set_frequency(uint16_t freq);
#endif

#endif /* _RTC_H */
