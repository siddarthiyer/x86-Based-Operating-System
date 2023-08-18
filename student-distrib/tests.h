#ifndef TESTS_H
#define TESTS_H

/************************* TESTING MACROS ********************************/
#define RUN_TESTS       1 // enable/disable the testing suite 

#if RUN_TESTS
#define RTC_WRITE_TEST  0 // sweep the rtc with different frequencies, and display output to the screen
#define RTC_SCREEN_TEST 0 // toggle define/undef to test the RTC handler with the test_interrupts video memory bump
#define RTC_PRINT_TIME  0 // toggle define/undef for RTC print time counter
#define PIT_PRINT_TIME  0
#else
#define RTC_WRITE_TEST  0
#define RTC_SCREEN_TEST 0
#define RTC_PRINT_TIME  0
#define PIT_PRINT_TIME  0
#endif

/*************************************************************************/

// test launcher
void launch_tests();

#endif /* TESTS_H */
