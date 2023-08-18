/** rtc.c
 *  Driver for the RTC (Real Time Clock)
*/
#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "syscall.h"
#include "terminal.h"
#include "bga.h"





/**************************** CONSTANTS **********************************/
/* value from 0 to 15, divides base frequency. Fastest value is 3, since 1,2 cause rollaround*/
#define RATE                  0x6                                   // Rate-Frequency relationship can be found at: https://www.ti.com/lit/ds/symlink/bq3285.pdf
#define RATE_FOR_2HZ          0xF                                   // the RATE which will produce 2Hz
#define HIGHEST_RTC_FREQUENCY 32768                                 // Highest theoretical frequency output by the RTC
#define MIN_RTC_FREQUENCY     2                                     // minumum frequency output by the RTC
#define REAL_FREQUENCY        (HIGHEST_RTC_FREQUENCY >> (RATE-1))   // the frequency which will be written to the rtc chip
/* PIC related constants*/
#define RTC_IRQ               8                                     // the RTC is IRQ 8 on the pic
/*************************************************************************/


/*********************** GLOBAL VARIABLES ********************************/
static          int32_t  term_freq[MAX_TERMINALS]  = {[0 ... (MAX_TERMINALS - 1)] = MIN_RTC_FREQUENCY};
static volatile int32_t  term_ticks[MAX_TERMINALS] = {[0 ... (MAX_TERMINALS - 1)] = 0};
static const    int32_t  real_frequency            = REAL_FREQUENCY;
static          int32_t  second_delay_ticks        = REAL_FREQUENCY;
static          int32_t  ms_delay_ticks            = REAL_FREQUENCY;
static          int32_t  cursor_delay_ticks        = (REAL_FREQUENCY)/2;
volatile static int8_t   ms_delay_flag             = 0;
volatile static int8_t   second_delay_flag         = 0;
static          uint8_t  cur_sec                   = 0;
static          uint8_t  last_sec                  = 0;
static struct   time_t   time                      = {0,0,0,0,sunday,1,1,0,20};
static volatile uint32_t ticks                     = 0;

int cursorflag = 1;

/*************************************************************************/


/*************************************************************************/
/****************** INITIALIZATION AND HANDLER **************************/
/*************************************************************************/
/** rtc_init 
 * DESCRIPTION: initialize the RTC, turn on periodic interrupts 
 * INPUTS:
 *      none
 * OUTPUTS:
 *      none
 * SIDE EFFECTS: RTC will begin generating interrupts at a rate of 1024 Hz.
 *               This rate can be modified by altering the RATE macro
 **/
void rtc_init()
{

    uint32_t previous_value;

    /* imperiative that NMI are disabled, otherwise potential to brick the CMOS timer */

    enable_irq(RTC_IRQ);

    /* set 4 LSB of register A to be rate */
    outb(REGISTER_A | DISABLE_NMI, INDEX);
    previous_value = inb(DATA);
    outb(REGISTER_A | DISABLE_NMI, INDEX);
    outb((previous_value & 0xF0) | RATE, DATA); // bit mask top 4

    /* select Flags Register and disable NMI. Register D selected by default */
    outb(REGISTER_B | DISABLE_NMI, INDEX);
    
    /* read current value of Flags*/
    previous_value = inb(DATA);

    /* select Flags Register again, since reading resets register to default D */
    outb(REGISTER_B | DISABLE_NMI, INDEX);

    /* write the old value of Flags to itsself, with the PIE bit enabled */
    outb(previous_value | PIE_ENABLE, DATA);

    /* enable NMI and reset register to default register D*/
    outb(REGISTER_D & ENABLE_NMI, INDEX);

}


/** rtc_handler
 * DESCRIPTION: handle each RTC interrupt.
 * INPUTS:
 *      none
 * OUTPUTS:
 *      none
 * SIDE EFFECTS: total ticks will be counted, and per file virtualization will be handled.
 *               Will toggle interrupt_received_flag upon per file virtualized interrupt.
*/
void rtc_handler(void)
{

    cli();

    /** read data from the RTC register C and discard it.
     *  This is needed so RTC interrupts are not blocked
     **/
    outb(REGISTER_C, INDEX);
    inb(DATA);

    static int i;
    /* decrement ticks for all terminal processes */
    for(i = 0; i < MAX_TERMINALS; i++)
    {
        term_ticks[i]--;
    }

    /* decrement global ticks and reset them after one second */
    ticks--;
    if(ticks == 0)
    {
        ticks = real_frequency;
    }

    /* decrement the counter for one second intervals */
    if(second_delay_ticks != 0)
    {
        second_delay_ticks--;
    }
    if(second_delay_ticks == 0)
    {
        second_delay_ticks = REAL_FREQUENCY;
        second_delay_flag = 1;
    }

    if(ms_delay_ticks != 0)
    {
        ms_delay_ticks--;
    }
    if(ms_delay_ticks == 0)
    {
        ms_delay_ticks = REAL_FREQUENCY;
        ms_delay_flag = 1;
    }


    if(cursor_delay_ticks != 0)
    {
        cursor_delay_ticks--;
        
    }
    if(cursor_delay_ticks == 0)
    {
        cursor_delay_ticks = (REAL_FREQUENCY)/4;

        if (cursorflag)
        {
            printLockCursor(0);   
            cursorflag = 0;
        }
        else
        {
            printLockCursor(1);
            cursorflag = 1;
        }
    }

    /* every once in a while (16 times per second), make sure the real-time is updated on the system */
    if(ticks % (REAL_FREQUENCY/16) == 0)
    {
        cur_sec = rtc_get_time_seconds();
        if(cur_sec != last_sec)
        {
            last_sec = cur_sec;
            printTime();
        }
    }


    // cursor_delay_ticks--;
    // if(cursor_delay_ticks == 0)
    // {
    //     cursor_delay_ticks = REAL_FREQUENCY/2;
        
    // }



    send_eoi(RTC_IRQ);
    sti();

}
/*************************************************************************/


/*************************************************************************/
/************************** Driver Functions *****************************/
/*************************************************************************/
/** rtc_read
 * DESCRIPTION: wait until the next RTC interrupt.
 * INPUTS:
 *      fd     - the file descriptor. Used to determine the rate of the interrupt per file,
 *               since different files can have different rates of interrupt.
 *      buf    - NA
 *      nbytes - NA
 * OUTPUTS: returns 0 after the next (virtualized) RTC interrupt
 * SIDE EFFECTS: blocks program from executing until an interrupt is generated.
*/
int rtc_read(int32_t fd, int8_t* buf, int32_t nbytes)
{    

    if(fd >= MAX_FD || fd < 0) return -1;

    // set ticks for current terminal 
    term_ticks[cur_term] = (real_frequency / 4) / term_freq[cur_term];

    // wait until the tick count down to zero
    while(term_ticks[cur_term] > 0);
    
    return 0;
}


/** rtc_write
 * DESCRIPTION: write a new frequency to the RTC. Frequency will be virtualized. For multiple processes, pass in a pid.
 * INPUTS: none
 * OUTPUTS: none
 * SIDE EFFECTS: the RTC interrupt rate will be set to that of the RTC buffer, if the buffer is a power of two
*/
int rtc_write(int32_t fd, const int8_t* buf, int32_t nbytes)
{
    cli();
    uint32_t buf_ = (uint32_t)*buf;

    /**
    *  if the value in the write buffer is not a power of two
    *  or the write buffer is greater than max frequency
    *  then return
    */
    if(fd >= MAX_FD) return -1;
    if(!((buf_ & (buf_ - 1)) == 0 && buf_ >= MIN_RTC_FREQUENCY)) return -1;
    if (buf_ > HIGHEST_RTC_FREQUENCY) return -1;
    
    // set the virtual frequency to the passed in freq
    term_freq[cur_term] = buf_;

    sti();
    
    return 0;
}


/** rtc_open
 * DESCRIPTION: reset the rate of the RTC to 2Hz
 * INPUTS: none
 * OUTPUTS: none
 * SIDE EFFECTS: RTC interrupt rate will be reset to 2Hz
*/
int rtc_open(const int8_t* filename)
{
    cli();

    // set the the virtual frequency to 2Hz
    term_freq[cur_term] = MIN_RTC_FREQUENCY;

    sti();

    return 0;
}


/** rtc_close 
 * DESCRIPTION: set the frequency for the associated file to -1
 * INPUTS: fd: file to close
 * OUTPUTS: none
 * SIDE EFFECTS: none
*/
int rtc_close(int32_t fd)
{
    return 0;
}


/** rtc_delay
 * DESCRIPTION: delay for a given amount of seconds
 * INPUTS:
 *      seconds: the amount of seconds to delay by
 * OUTPUTS: none
 * SIDE EFFECTS: block execution on the code that calls this function for input amount of second
*/
void rtc_delay(uint8_t seconds)
{
    static int s;
    for(s = 0; s < seconds; s++)
    {
        second_delay_ticks = real_frequency/MIN_RTC_FREQUENCY;
        while(!second_delay_flag);
        second_delay_flag = 0;
    }
}


/** rtc_delay_inv
 * DESCRIPTION: delay for 1/input seconds
 * INPUTS:
 *      delay: how long to delay for. delay is 1/input seconds. must be a power of 2
 * OUTPUTS: none
 * SIDE EFFECTS: block execution on the code that calls this function for 1/input amount of second
*/
void rtc_delay_inv(uint8_t inv_seconds)
{

    /* ensure input is a power of two */

    ms_delay_ticks = (real_frequency/inv_seconds) / MIN_RTC_FREQUENCY;
    while(!ms_delay_flag);
    ms_delay_flag = 0;
}


/** rtc_get_time
 * DESCRIPTION: get the current date and time
 * INPUTS: none
 * OUTPUTS: a time structure containing the current date and time
 * SIDE EFFECTS: none
*/
struct time_t* rtc_get_time()
{

    /** time is output from the CMOS in BCD mode, meaning a conversion to
     *  decimal is needed. BCD mode outputs all hex values in decimal-like format.
     *  EXAMPLE: 59 seconds -> 0x59 rather than 0x3B
    */

    static struct time_t time = {0,0,0,0,sunday,1,1,0,20};

    static int delay;
    static int temp;

    outb(REGISTER_SECONDS & ENABLE_NMI, INDEX);
    temp = inb(DATA);
    time.seconds = BCD_TO_DECIMAL(temp);

    for(delay = 0; delay < 20; delay++);

    outb(REGISTER_MINUTES & ENABLE_NMI, INDEX);
    temp = inb(DATA);
    time.minutes = BCD_TO_DECIMAL(temp);

    for(delay = 0; delay < 20; delay++);

    /* hours are in 24 hour format, for some reason CMOS is 5 hours ahead... */
    outb(REGISTER_HOURS & ENABLE_NMI, INDEX);
    temp = inb(DATA);
    time.hours = BCD_TO_DECIMAL(temp) - 5;
    if(time.hours > 24)
    {
        time.hours += 5;
    }

    for(delay = 0; delay < 20; delay++);
    
    outb(REGISTER_WEEKDAY & ENABLE_NMI, INDEX);
    time.weekday = inb(DATA);

    for(delay = 0; delay < 25; delay++);

    outb(REGISTER_DAY_OF_MONTH & ENABLE_NMI, INDEX);
    temp = inb(DATA);
    time.day_of_month = BCD_TO_DECIMAL(temp);

    for(delay = 0; delay < 20; delay++);

    outb(REGISTER_MONTH & ENABLE_NMI, INDEX);
    temp = inb(DATA);
    time.month = BCD_TO_DECIMAL(temp);

    for(delay = 0; delay < 20; delay++);

    outb(REGISTER_YEAR & ENABLE_NMI, INDEX);
    temp = inb(DATA);
    time.year = BCD_TO_DECIMAL(temp);

    /* omit century for now. */
    /** TODO: add logic for century within the next 80 years */ 

    return &time;
}


/** rtc_get_time_seconds
 * DESCRIPTION: get the time's seconds
 * INPUTS: none
 * OUTPUTS: the current seconds
 * SIDE EFFECTS: none
*/
uint8_t rtc_get_time_seconds()
{
    static uint8_t temp;

    outb(REGISTER_SECONDS & ENABLE_NMI, INDEX);
    temp = inb(DATA);
    return BCD_TO_DECIMAL(temp);
}
/*************************************************************************/
