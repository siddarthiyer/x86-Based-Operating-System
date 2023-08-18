/** pit.h
 * driver for the pit (Programmable Interval Timer)
*/

#include "types.h"

#ifndef _PIT_H
#define _PIT_H


/******************************************* UTILITY MACROS **************************************************/
#define ENABLE_SOUND 1      // Enable/Disable the Sound Blaster
/*****************************************************************************************************/



/** BACKGROUND:
 *  - The oscillator used by the PIT chip runs at (roughly) 1.193182 MHz
 *  - The PIT has only 16 bits that are used as frequency divider, which can represent the values from 0 to 65535.
 *  - The PIT chip has three separate frequency dividers (or 3 separate channels) that are programmable
 **/
#define IRQ0_BASE_FREQUENCY 1193180 // the frequency at which pit generates an IRQ0 Int
#define PIT_IRQ             0

/** CHANNELS:
 * CHANNEL 0: 
 *    ! - The output from PIT channel 0 is connected to the PIC chip, so that it generates an "IRQ 0"
 *      - Typically during boot the BIOS sets channel 0 with a count of 65535 or 0, which gives an output frequency of 18.2065 Hz.
 *      - Slowest Frequency -> 18.2065 Hz (or an IRQ every 54.9254 ms)
 * CHANNEL 1: 
 *      - Obsolete
 *      - used to refresh the DRAM
 * CHANNEL 2:
 *    ! - Connected to PC speaker
 *      - frequency of the output determines the frequency of the sound produced by the speaker.
 *      
*/

/** PORTS:
 * - Each 8 bit data port is the same, and is used to set the counter's 16 bit reload value or read the channel's 16 bit current count
*/
#define CHAN_0_DATA_PORT   0x40
#define CHAN_1_DATA_PORT   0x41
#define CHAN_2_DATA_PORT   0x42


/** Channel 2's Gate Input and Output Value can be read through port 0x61
 * 
 * Gate Input -> BIT 0: enable/disable wether the signal reaches the speaker
 * Output     -> BIT 5: output voltage, high or low
*/
#define CHAN_2_RW_PORT     0x61     // gate can be enabled / disabled through bit 0


/** CMD:
 * Bits         Usage
 * 6 and 7      Select channel :
 *                0 0 = Channel 0
 *                0 1 = Channel 1
 *                1 0 = Channel 2
 *                1 1 = Read-back command (8254 only)
 * 4 and 5      Access mode :
 *                0 0 = Latch count value command
 *                0 1 = Access mode: lobyte only
 *                1 0 = Access mode: hibyte only
 *                1 1 = Access mode: lobyte/hibyte
 * 1 to 3       Operating mode :
 *                0 0 0 = Mode 0 (interrupt on terminal count) -> only generates interrupts on Channel 0
 *                0 0 1 = Mode 1 (hardware re-triggerable one-shot)
 *                0 1 0 = Mode 2 (rate generator)
 *                0 1 1 = Mode 3 (square wave generator)
 *                1 0 0 = Mode 4 (software triggered strobe)
 *                1 0 1 = Mode 5 (hardware triggered strobe)
 *                1 1 0 = Mode 2 (rate generator, same as 010b)
 *                1 1 1 = Mode 3 (square wave generator, same as 011b)
 * 0            BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD
 *                  -> 80x86 PCs only use binary mode
 **/
#define MODE_CMD_PORT      0x43     // w (read ignored)

#define SELECT_CHANNEL_0         0x00
#define SELECT_CHANNEL_2         0x80
#define ACCESS_MODE_HIBYTE       0x20
#define ACCESS_MODE_LOBYTE_HBYTE 0x30
#define OPERATING_MODE_0         0x00 // used for IRQ0
#define OPERATING_MODE_SQR_WAVE  0x06 // used for speakers
#define BINARY_MODE              0x00


/************ Calculation Macros ****************/
#define ms(x)           1000/x    /* convert hertz to milliseconds */
#define high_byte(x)    (x >> 8)
#define low_byte(x)     (x & 0xFF)
/************************************************/



void pit_init();
void pit_handler();


void play_sound(uint32_t nFrequency);
void speaker_beep(uint32_t nFrequency);
void speaker_mute();

#endif // _PIT_H
