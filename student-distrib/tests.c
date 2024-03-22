#include "tests.h"
#include "x86_desc.h"
#include "file_system.h"
#include "terminal.h"
#include "rtc.h"
#include "ata.h"

#define PASS 1
#define FAIL 0

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
// int page_deref_null(){
// 	TEST_HEADER;

// 	uint8_t* ptr = NULL;
// 	uint8_t test = *ptr; //should generate an exception
// 	return FAIL;
// }


int page_deref_kernel_start(){
	TEST_HEADER;

	uint8_t* ptr = (uint8_t*)0x400000;
	printf("kernel starts with value: %x\n at: %x\n", *ptr, ptr);
	// if (*ptr != 0x400000)
	// 	return FAIL;
	return PASS;
}

int page_deref_kernel_end(){
	TEST_HEADER;

	uint8_t* ptr = (uint8_t*)0x7FFFFF;
	printf("kernel ends with value: %x\n at: %x\n", *ptr, ptr);
	return PASS;
}

int page_deref_vidmem_starts(){
	TEST_HEADER;

	uint8_t* ptr = (uint8_t*)0xB8000;
	printf("video memory starts with value: %x\n at: %x\n", *ptr, ptr);
	return PASS;
}

int page_deref_vidmem_ends(){
	TEST_HEADER;

	uint8_t* ptr = (uint8_t*)0xB8FFF;
	printf("video memory ends with value: %x\n at: %x\n", *ptr, ptr);
	return PASS;
}

int page_deref_noshow1(){
	TEST_HEADER;

	uint8_t* ptr = (uint8_t*)0x100000; //beyond 4MB
	printf("can't access this address: %x\n", *ptr);
	return PASS;
}

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
	printf("idt[0] = %x\n", idt[0].offset_15_00);
	return result;
}

// add more tests here
int divide_zero_test(){
	TEST_HEADER;

	int a = 1;
	int b = 0;
	int c = a / b;
	printf("c = %d\n", c);
	return PASS;
}

int test_systemcall(){
	TEST_HEADER;

	__asm__("int $0x80");
	return PASS;
}

/* test_rtc()
 * Inputs: none
 * Return Value: none
 * Function: Increments video memory to check responses to RTC interrupts.
 */
extern void test_interrupts();
int test_rtc() {
	while (1) {
		test_interrupts();
	}//defined in lib.c, check if the video can respond to RTC interrupts
	return PASS;
}



/* Checkpoint 2 tests */

/*
* File Open test
* Asserts we can open the file
* Inputs: None
* Outputs: PASS/FAIL
* Side Effects: None
* Coverage: file_system.c/h
*/
int open_file_test(uint8_t* filename){
	TEST_HEADER;
	if (file_open(filename) == -1){
		return FAIL;
	}	
	return PASS;
}

/*
* File Close test
* Asserts we can close the file
* Inputs: None
* Outputs: PASS/FAIL
* Side Effects: None
* Coverage: file_system.c/h
*/
int close_file_test(){
	TEST_HEADER;
	int fd = 0;
	if (file_close(fd) == 0)
		return PASS;
	return FAIL;
}

/*
* File Read test
* Asserts we can read the file
* Inputs: None
* Outputs: PASS/FAIL
* Side Effects: None
* Coverage: file_system.c/h
*/
int read_file_test(){
	TEST_HEADER;
	if (file_read(0, NULL, 0) == 0){
		return PASS;
	}
	return FAIL;
}

/* 
Directory Read Test - mimics ls command (by name) <--- USE THIS ONE
 * 
 * Inputs: None
 * Outputs: Should print all known files to the screen ls style
 * Side Effects: None
 * Coverage: Directory reading functionality + read_dentry_by_index helper func
 * Files: file_system.h, file_system.c
 */
int directory_read_test()
{
	clear();
	uint8_t buf[4096];

	// ignore nbytes and fd (unused in this test)
	directory_open((uint8_t *)".");
	directory_read(0, buf, 64);					
	directory_open((uint8_t *)"sigtest");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"shell");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"grep");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"syserr");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"rtc");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"fish");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"counter");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"pingpong");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"cat");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"frame0.txt");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"verylargetextwithverylongname.tx");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"ls");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"testprint");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"created.txt");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"frame1.txt");
	directory_read(0, buf, 64);
	directory_open((uint8_t *)"hello");
	directory_read(0, buf, 64);
	
	return PASS;
}

int read_frame0_txt_test() {
	clear();
	// TEST_HEADER;
	dentry_t test;
	char buff[40000] = {'\0'};
	int i;
	int byte_counter;
	read_dentry_by_name((uint8_t*)"frame0.txt",&test);
	// 187 is the length of "frame0.txt" file
	byte_counter = read_data(test.inode_num,0,(uint8_t*)buff,10000); 
	// printf("read %d bytes\n",byte_counter);
	for(i=0; i < byte_counter; i++){
		putc(buff[i]);
	}
	return PASS;
}

int read_ls_EIF_test() {
	// TEST_HEADER;
	dentry_t test;
	char buff[5349] = {'\0'};
	int i;
	int byte_counter;
	read_dentry_by_name((uint8_t*)"ls",&test);
	printf("read dentry by name work good\n");
	// 5349 is the length of "ls" file
	byte_counter = read_data(test.inode_num,0,(uint8_t*)buff,5349); 

	clear();
	// printf("read %d bytes\n",byte_counter);
	// is enough to show the ELF
	for(i = 0; i < 10; i++){
		putc(buff[i]);
	}
	return PASS;
}

int read_ls_string_test() {
	// TEST_HEADER;
	dentry_t test;
	char buff[5349] = {'\0'};
	int i;
	int byte_counter;
	read_dentry_by_name((uint8_t*)"ls",&test);
	printf("read dentry by name work good\n");
	// 5349 is the length of "ls" file
	byte_counter = read_data(test.inode_num,0,(uint8_t*)buff,5349); 

	clear();
	// printf("read %d bytes\n",byte_counter);

	// is enough to show the magic string "01234..."
	for(i = 5300; i < 5349; i++){
		putc(buff[i]);
	}
	return PASS;
}

int read_large_file_test() {
	// TEST_HEADER;
	dentry_t test;
	char buff[5277] = {'\0'};
	int i;
	int byte_counter;
	read_dentry_by_name((uint8_t*)"verylargetextwithverylongname.tx",&test);
	byte_counter = read_data(test.inode_num,0,(uint8_t*)buff,5277); 
	clear();
	printf("read %d bytes\n",byte_counter);
	for(i=0; i < byte_counter; i++){
		putc(buff[i]);
	}
	return PASS;
}

/*
* Keyboard read & write test
* Asserts we can read and write certain number of bytes successfully
* Inputs: Number of bytes we should read or write
* Outputs: PASS/FAIL
* Side Effects: None
* Coverage: terminal.c
*/
int terminal_read_test(int32_t read_nbytes, int32_t write_nbytes) {
	//TEST_HEADER;

	int32_t retr, retw;
	uint8_t buf[128];
	while (1) {
		retr = terminal_read(0, buf, read_nbytes);
		retw = terminal_write(0, buf, write_nbytes);
		printf("Read %d bytes, wrote %d bytes\n", retr, retw);
	}

	return PASS;
}

/*
* Terminal_write write a new line 
* Asserts we can add a new line and scrolling the screen successfully
* Inputs: Number of bytes we should write
* Outputs: PASS/FAIL
* Side Effects: None
* Coverage: terminal.c
*/
int terminal_write_test(int32_t write_nbytes) {
	//TEST_HEADER;

	uint8_t buffer[100] = {
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		'!', '@', '#', '%', '^', '&', '*', '(', ')', '$',
		'_', '-', '=', '+', '[', ']', '{', '}', '\\', '|',
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		'a', 'b', '~', 'd', 'e', 'U', ';', ':', ' ', 'g',
		'6', '3', '`', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		'a', 'b', 'D', 'd', 'e', '>', 'g', 'h', 'D', 'j',
		'3', 'b', 'c', 'd', 'N', 'f', 'E', 'S', 'i', 'j',
		'Q', 'B', 'c', 'd', 'e', 'f', 'g', 'P', 'O', '<',
		'a', 'b', 'Z', ',', 'N', 'f', 'G', 'h', 'i', 'j',
	};
	terminal_init();
	terminal_write(0, buffer, write_nbytes);

	printf("\n");

	return PASS;
}

/*
* Terminal_write buffer NULL
* Asserts it can show "terminal_write_fails" directly
* Inputs: Number of bytes we should write
* Outputs: PASS/FAIL
* Side Effects: None
* Coverage: terminal.c
*/
int terminal_write_null_test(int32_t write_nbytes) {
	//TEST_HEADER;

	uint8_t *buffer = NULL;

	terminal_init();
	terminal_write(0, buffer, write_nbytes);

	printf("\n");

	return PASS;
}

/* test_rtc_2()
 * Inputs: none
 * Return Value: none
 * Function: Changing RTC frequencies, receiving RTC interrupts at each possible frequency
 * Coverage: rtc.c
 */
int rtc_test_orw() {
	TEST_HEADER;
	clear();
	int i, frequency;//initialize iterator and test frequency
	printf("test rtc_open:\n");
	rtc_open(NULL);
	for (i = 0; i < 2; i++) {
		rtc_read(0, NULL, 0);//wait in the frequency specified by rtc_open()
		printf("2");//frequency specified by rtc_open is 2
	}
	
	printf("\n");

	for (frequency = 2; frequency <= 1024; frequency *= 2) {//traverse all frequencies
		printf("current frquency is %d\n", frequency);
		rtc_write(0, &frequency, 4);//frequecy is a 4 byte variable
		for (i = 0; i < frequency; i++) {//compare the frequencies according to the waiting time needed
			// printf("read start\n");
			rtc_read(0, NULL, 0);//wait in the specific frequency
			// printf("read end\n");
			printf("%d", frequency);
		}
		printf("\n");
	}
	return PASS;//return PASS if success
}

//check if rtc_close is successful
int rtc_test_close() {
	TEST_HEADER;
	clear();
	if (0 == rtc_close(0)) printf("RTC_CLOSE PASS");
	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

int ata_read_test() {
	TEST_HEADER;
	int32_t i, j;
	int8_t flag;
	uint8_t buf[512] = {0};
	buf[0] = 1; 
	printf("buf created:\n");
	for (i = 0; i < 512; i++) {
		printf("%x", buf[i]);
	}
	printf("\n");
	flag = ata_read_pio28(buf, 1, 0);
	printf("buf after reading:\n");
	for (j = 0; j < 512; j++) {
		printf("%x", buf[j]);
	}
	printf("\n");
	return flag;
}

int ata_write_test() {
	TEST_HEADER;
	uint32_t i, j;
	uint8_t flag;
	uint8_t buf_read[512] = { 0 };
	uint8_t buf_write[512];
	uint32_t target_sec = 5000;

	ata_read_pio28(buf_read, target_sec, 1);
	printf("buf before writing:\n");
	for (i = 0; i < ATA_SECTOR_SIZE; i++) {
		printf("%x", buf_read[i]);
	}
	printf("\n");

	for (j = 0; j < ATA_SECTOR_SIZE; j += 4) {
		buf_write[j] = 0x12;
		buf_write[j+1] = 0x34;
		buf_write[j+2] = 0x56;
		buf_write[j+3] = 0x78;
	}

	flag = ata_write_pio28(buf_write, target_sec, 1);
	printf("reading ...\n");
	ata_read_pio28(buf_read, target_sec, 1);
	printf("buf after writing:\n");
	for (i = 0; i < ATA_SECTOR_SIZE; i++) {
		printf("%x", buf_read[i]);
	}
	printf("\n");
	return flag;
}

int file_create_test(){
	TEST_HEADER;
	uint8_t buf[32] = {"good"};
	int32_t ret;
	dentry_t dentry;
	int32_t i;
	ret = file_create(buf);
	ret = read_dentry_by_name (buf,&dentry);
	printf("filename is ");
	for (i = 0; i < 4; i++){
		printf("%c",dentry.filename[i]);
	}
	printf("\nfile inode is %d\n",dentry.inode_num);
	return ret;
}

/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	// TEST_OUTPUT("divide_zero_test", divide_zero_test());
	// TEST_OUTPUT("page_deref_null", page_deref_null());
	// TEST_OUTPUT("page_deref_no_show1", page_deref_noshow1());
	//this three will directly crash the machine!
	
	
	// TEST_OUTPUT("test_rtc", test_rtc());
	// TEST_OUTPUT("test_rtc", test_rtc());
	// TEST_OUTPUT("test_systemcall",test_systemcall());

	//check validity
	// TEST_OUTPUT("page_deref_kernel_start", page_deref_kernel_start());
	// TEST_OUTPUT("page_deref_kernel_end", page_deref_kernel_end());
	// TEST_OUTPUT("page_deref_vidmem_starts", page_deref_vidmem_starts());
	// TEST_OUTPUT("page_deref_vidmem_ends", page_deref_vidmem_ends());

	//check file system
	// TEST_OUTPUT("open_file_test", open_file_test((uint8_t*)"frame0.txt"));
	// TEST_OUTPUT("close_file_test", close_file_test());
	// TEST_OUTPUT("read_file_test", read_file_test());
	// TEST_OUTPUT("read_directory_test",directory_read_test());
	// TEST_OUTPUT("read_frame0_txt_test",read_frame0_txt_test());
	// TEST_OUTPUT("read_ls_EIF_test",read_ls_EIF_test());
	// TEST_OUTPUT("read_ls_string_test",read_ls_string_test());
	// TEST_OUTPUT("read_large_file_test",read_large_file_test());

	// check terminal 
	//TEST_OUTPUT("terminal read test", terminal_read_test(128, 128));
	//TEST_OUTPUT("terminal_write_test", terminal_write_test(100));
	//TEST_OUTPUT("terminal_write_null_test", terminal_write_null_test(100));

	// check rtc(checkpoint2)
	//TEST_OUTPUT("rtc_test_orw", rtc_test_orw());
	// TEST_OUTPUT("rtc_test_close", rtc_test_close());

	//check ata
	// TEST_OUTPUT("ata_read_test", ata_read_test());
	// TEST_OUTPUT("ata_write_test", ata_write_test());
	TEST_OUTPUT("file create test", file_create_test());
	
}
