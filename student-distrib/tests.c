#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "terminal.h"
#include "fs_driver.h"

#define PASS 1
#define FAIL 0

#define MAX_FILE_SIZE	36164	

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Paging Test
 * 
 * Accesses valid and invalid pages and checks if page fault occurs
 * Inputs: None
 * Outputs: PASS/Page Fault
 * Side Effects: May cause Page Fault
 * Coverage: Paging, Page Fault exception
 * Files: paging.h/S/C
 */
int paging_test(){
	TEST_HEADER;
	uint8_t data;
	uint8_t * addr;

	addr = (uint8_t *)0xb8000;	// Valid
	data = *addr;

	addr = (uint8_t *)0xb8fff;	// Valid
	data = *addr;

	addr = (uint8_t *)0x400000;	// Valid	
	data = *addr;

	addr = (uint8_t *)0x7fffff;	// Valid	
	data = *addr;

	// addr = (uint8_t *)0xb7fff; 	// Invalid
	// data = *addr;

	// addr = (uint8_t *)0xb9000;	// Invalid
	// data = *addr;

	// addr = (uint8_t *)0x3FFFFF;	// Invalid
	// data = *addr;

	// addr = (uint8_t *)0x800000;	// Invalid
	// data = *addr;

	// addr = (uint8_t *)0;	// Invalid
	// data = *addr;

	return PASS;
}

/* Syscall Test
 * 
 * Triggers a syscall
 * Inputs: None
 * Outputs: Sys Call excpetion occurs
 * Side Effects: Causes sys call
 * Coverage: Sys call, IDT
 * Files: inthandlers.c/h
 */
int syscall_test(){
	TEST_HEADER;
	__asm__(" movl $0x0, %eax \n\
	int $0x21");
	// __asm__("int	$0x80");
	printf(" You should see proper syscall executed \n");
	return PASS;
}

/* Syscall Test
 * 
 * Triggers a divide by 0 exception
 * Inputs: None
 * Outputs: divide by  excpetion occurs
 * Side Effects: Causes exception
 * Coverage: Exceptions, IDT
 * Files: inthandlers.c/h
 */
int divide_by_zero_test(){
	TEST_HEADER;
	int x = 1;
	int y = 0;
	int z;
	z = x/y;
	return FAIL; 
}

// #if RTC_WRITE_TEST
// /**
//  * DESCRIPTION: tests all driver functions of RTC (open, close, read, write)
//  * 				and sets the RTC to multiple frequencies
// */
// void rtc_change_frequency_test()
// {
// 	uint16_t i = 0;

// 	rtc_close();

//     rtc_open();
// 	printf("rtc_read called\n");
// 	for(i = 0; i < 10; i++)
// 	{
// 		rtc_read();
// 	}
// 	printf("\n");

// 	printf("changing frequency to 4Hz...\n");
// 	set_frequency(4);
// 	rtc_write();
// 	printf("rtc_write called\n");
// 	for(i = 0; i < 10; i++)
// 	{
// 		rtc_read();
// 	}
// 	printf("\n");

// 	printf("changing frequency to 8Hz...\n");
// 	set_frequency(8);
// 	rtc_write();
// 	for(i = 0; i < 10; i++)
// 	{
// 		rtc_read();
// 	}
// 	printf("\n");

// 	printf("changing frequency to 16Hz...\n");
// 	set_frequency(16);
// 	rtc_write();
// 	for(i = 0; i < 10; i++)
// 	{
// 		rtc_read();
// 	}
// 	printf("\n");

// 	printf("changing frequency to 32Hz...\n");
// 	set_frequency(32);
// 	rtc_write();
// 	for(i = 0; i < 10; i++)
// 	{
// 		rtc_read();
// 	}
// 	printf("\n");

// 	printf("changing frequency to 64Hz...\n");
// 	set_frequency(64);
// 	rtc_write();
// 	for(i = 0; i < 10; i++)
// 	{
// 		rtc_read();
// 	}
// 	printf("\n");

// 	printf("changing frequency to 128Hz...\n");
// 	set_frequency(128);
// 	rtc_write();
// 	for(i = 0; i < 10; i++)
// 	{
// 		rtc_read();
// 	}
// }
// #endif
// #if RTC_PRINT_TIME
// void rtc_print_time_test()
// {
// 	while(1)
// 	{
// 		rtc_read();
// 	}
// }
// #endif

// /* Checkpoint 2 tests */

// /* terminal_test
//  * 
//  * Tests terminal
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: N/A
//  * Coverage: Terminal
//  * Files: terminal.h/c
//  */
// int terminal_test(){
// 	TEST_HEADER;
// 	int i;
// 	char testbuf[129];	// 129-char test buffer
// 	int status;
// 	while(strncmp(testbuf, "EXIT", 4) != 0){
// 		int lastidx;
// 		for(i = 0; i < 129; i++){ // clear test buffer
// 			testbuf[i] = 0;
// 		}
// 		printf("time is: %d, type something followed by ENTER:\n", rtc_read());
// 		printf("('EXIT' will quit this test)\n");
// 		status = terminal_write(0, "input>DONTPRINTTHIS", 6);
// 		if(status != 6){
// 			return FAIL;	// write didn't behave as expected!
// 		}
// 		status = terminal_read(0, testbuf, 128);
// 		if(status < 0){
// 			return FAIL;	// read messed up!
// 		}
// 		printf("printing results of %d bytes read in terminal below (strlen is %d):\n",
// 			 status, strlen(testbuf));
// 		terminal_write(0, testbuf, 129);
// 		lastidx = strlen(testbuf)-1;
// 		if(lastidx >= 128){
// 			lastidx = 127;
// 		}
// 		if(testbuf[lastidx] != '\n'){
// 			printf("output didn't end in newline character!\n");
// 			return FAIL;	// must end in \n
// 		}
// 		if(testbuf[lastidx - 1] == '\n'){
// 			printf("output had too many newline characters!\n");
// 			return FAIL;	// too many \n!
// 		}
// 		if(testbuf[128] != '\0'){
// 			printf("more than 128 characters returned!");
// 			return FAIL;
// 		}
// 	}
// 	printf("testing some erroneous inputs, should be 4 errors:\n");
// 	status = 0;
// 	status += terminal_write(0, 0, 5);
// 	status += terminal_write(0, 0, -3);
// 	status += terminal_read(0, 0, 5);
// 	status += terminal_read(0, 0, -3);
// 	if(status != -4){
// 		return FAIL;	// input checking failed
// 	}
// 	return PASS;
// }

// int fs_test1(){
// 	dentry_t d;
//     uint32_t inode;
//     inode = 0;
//     int32_t da;
//     uint8_t buf[MAX_FILE_SIZE];

//     if (read_dentry_by_name((uint8_t *)"verylargetextwithverylongname.tx", &d) == 0)
// 	{
//         inode = d.inode_num;
// 		printf("file_name: ");
//         printf(d.filename);
//     }
//     else
// 	{
//         return FAIL;
//     }
//     da = read_data(inode, 0, buf, MAX_FILE_SIZE);
//     printf("     bytes_read: %d\n", da);
//     int i;
//     for (i = 0; i < da; i++)
// 	{
//         putc(buf[i]);
//     }
// 	return PASS;
// }

/* fs_file_open_test
 * 
 * Calls file open on files
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: N/A
 * Coverage: File Systems
 * Files: fs_driver.h/c
 */
// int fs_file_open_test(){
// 	TEST_HEADER;
// 	int i;
// 	for (i = 0; i < MAX_FD; i++)
// 	{
// 		fdt[i].fot_ptr = 0;
// 		fdt[i].inode = 0;
// 		fdt[i].file_pos = 0;
// 		fdt[i].flag = 0;
// 	}

// 	uint8_t buf[MAX_FILE_SIZE];

// 	// int fd = file_open((uint8_t *)"frame0.txt");
// 	// int fd = file_open((uint8_t *)"frame1.txt");
// 	// int fd = file_open((uint8_t *)"verylargetextwithverylongname.txt");
// 	int fd = file_open((uint8_t *)"verylargetextwithverylongname.tx");
// 	int len = file_read (fd, buf, MAX_FILE_SIZE);
// 	if (len == -1) return FAIL;
// 	for (i = 0; i < len; i++) {
//         putc(buf[i]);
//     }
// 	printf("\nfile_size: %d\n", len);
// 	return PASS;
// }

// /* fs_exec_open_test
//  * 
//  * Calls file open on exec files
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: N/A
//  * Coverage: File Systems
//  * Files: fs_driver.h/c
//  */
// int fs_exec_open_test(){
// 	TEST_HEADER;
// 	int i;
// 	for (i = 0; i < MAX_FD; i++)
// 	{
// 		fdt[i].fot_ptr = 0;
// 		fdt[i].inode = 0;
// 		fdt[i].file_pos = 0;
// 		fdt[i].flag = 0;
// 	}

// 	uint8_t buf[MAX_FILE_SIZE];

// 	// int fd = file_open((uint8_t *)"grep");
// 	int fd = file_open((uint8_t *)"ls");
// 	// int fd = file_open((uint8_t *)"fish");
// 	int len = file_read (fd, buf, MAX_FILE_SIZE);
// 	if (len == -1) return FAIL;
// 	for (i = 0; i < len; i++) {
// 		if (buf[i] != 0)
//         putc(buf[i]);
//     }
// 	printf("\nfile_size: %d\n", len);
// 	return PASS;
// }

// /* fs_exec_open_test
//  * 
//  * Calls dir open on all directories
//  * Inputs: None
//  * Outputs: PASS/FAIL- similar to ls
//  * Side Effects: N/A
//  * Coverage: File Systems
//  * Files: fs_driver.h/c
//  */
// int dir_test(){
// 	TEST_HEADER;
//     uint8_t fname[MAX_FILE_NAME];
//     int32_t len;
// 	int i;

// 	for (i = 0; i < MAX_FD; i++)
// 	{
// 		fdt[i].fot_ptr = 0;
// 		fdt[i].inode = 0;
// 		fdt[i].file_pos = 0;
// 		fdt[i].flag = 0;
// 	}

//     int fd = dir_open((uint8_t *)".");
// 	printf("%d \n", fd);

//     for (i = 0; i < g_dir_count; i++)
// 	{
//         len = dir_read(fd, fname, 0);
//         printf("file_name: ");
// 		dentry_t d;
// 		read_dentry_by_name (fname, &d);
// 		int type = d.filetype;
// 		inode_t  * cur_inode = (inode_t *)(inode_ptr + d.inode_num * BLOCK_SIZE);
// 		int size = cur_inode->length;
// 		int j;
//         for (j = 0; j < len; j++)
// 		{
//         	putc(fname[j]);
//     	}
// 		printf(", file_type: %d, file_size: %d", type, size);
//         printf("\n");
//     }
// 	return PASS;
// }
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("paging_test", paging_test());

	// rtc_read();

	// TEST_OUTPUT("divide_by_zero_test", divide_by_zero_test());
	// TEST_OUTPUT("syscall_test", syscall_test());
	// launch your tests here
	

	/*Checkpoint 2*/

	// TEST_OUTPUT("terminal_test", terminal_test());

	#if RTC_WRITE_TEST
		// rtc_change_frequency_test();	// test toggle in rtc.h
	#endif
	#if RTC_PRINT_TIME
		rtc_print_time_test();
	#endif

	// TEST_OUTPUT("dir_test", dir_test());
	// TEST_OUTPUT("fs_exec_open_test", fs_exec_open_test());
	// TEST_OUTPUT("fs_file_open_test", fs_file_open_test());


}
