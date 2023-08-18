/** pit.c
 * driver for the pit (Programmable Interval Timer)
*/

#include "pit.h"
#include "lib.h"
#include "i8259.h"
#include "tests.h"
#include "scheduler.h"
#include "rtc.h"

static uint16_t frequency_divider;   // 16-bit value from 0 to 65535. Divides base frequency to get lower frequency.

#define IRQ0_MS             25      // number of milliseconds between IRQ0 ints

/** pit_init
 * DESCRIPTION: initialize the PIT
 * INPUTS:  none
 * OUTPUTS: none
 * SIDE EFFECTS: 
 *      - PIT will generate interrupts every 10 to 50 ms
*/
void pit_init()
{
    uint8_t write_byte;

    /* set the frequency divider to correspond to 25ms interrupts (47727) */
    frequency_divider = IRQ0_BASE_FREQUENCY / IRQ0_MS;

    /* configure Channel 0 and write to CMD port */
    write_byte = SELECT_CHANNEL_0 | ACCESS_MODE_LOBYTE_HBYTE | OPERATING_MODE_0 | BINARY_MODE;
    outb(write_byte, MODE_CMD_PORT);

    /* set the reload register on channel 0 to frequency_divider */
    outb(low_byte(frequency_divider), CHAN_0_DATA_PORT);
    outb(high_byte(frequency_divider), CHAN_0_DATA_PORT);

    /* configure PIT to acknowledge interrupts in only one command */
    outb(SELECT_CHANNEL_0 | ACCESS_MODE_HIBYTE | OPERATING_MODE_0 | BINARY_MODE, MODE_CMD_PORT);

    enable_irq(PIT_IRQ);

}


/** pit_handler
 * DESCRIPTION: call scheduler on each interrupt
 * INPUTS: none
 * OUTPUTS: none
 * SIDE EFFECTS: none
*/
void pit_handler()
{

    cli();

    /** PIT is configured to ACCESS_MODE_HIBYTE.
     *  acknowledge the interrupt by resetting the Channel 0 reload value high byte */
    outb(high_byte(frequency_divider), CHAN_0_DATA_PORT);

    send_eoi(PIT_IRQ);
    sti();

    task_switch();

}



#if ENABLE_SOUND

/** play_sound
 * DESCRIPTION: play a sound on the speaker at a given frequency
 * INPUTS: 
 *      nFrequency: the frequency at which to play the sound
 * OUTPUTS: none
 * SIDE EFFECTS: none
*/
void play_sound(uint32_t nFrequency)
{
    uint32_t divisor;
    uint8_t  read;
    uint8_t  write;

    /* determine the frequency divisor to write to the PIT*/
    divisor = IRQ0_BASE_FREQUENCY / nFrequency;

    /* select and configure channel 2 of the PIT */
    write = SELECT_CHANNEL_2 | ACCESS_MODE_LOBYTE_HBYTE | OPERATING_MODE_SQR_WAVE;

    outb(write, MODE_CMD_PORT);

    /* write the frequency to the PIT */
    outb((uint8_t)low_byte(divisor), CHAN_2_DATA_PORT);
    outb((uint8_t)high_byte(divisor), CHAN_2_DATA_PORT);

    /* play the sound using the PC speaker */
    read = inb(CHAN_2_RW_PORT);
    if(read != (read | 0x3))
    {
        outb(read | 0x3, CHAN_2_RW_PORT);
    }
}

/** speaker_beep
 * DESCRIPTION: play an audible beep through the computer speaker
 * INPUTS: none
 * OUTPUTS: none
*/
void speaker_beep(uint32_t nFrequency)
{
    play_sound(nFrequency);
    rtc_delay_inv(6);
    speaker_mute();

}

/** speaker_mute
 * DESCRIPTION: mute the PC speaker
 * INPUTS: none
 * OUTPUTS: none
*/
void speaker_mute()
{
    uint8_t port_val;

    port_val = inb(CHAN_2_RW_PORT);
    
    /* mute speaker by setting output bit to 0 */
    outb(port_val & 0xFC, CHAN_2_RW_PORT);
}

#endif
