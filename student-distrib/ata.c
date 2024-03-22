#include "ata.h"
#include "types.h"
#include "lib.h"

/*whole ata driver is based on os.dev:
https://wiki.osdev.org/ATA_PIO_Mode#Cache_Flus
*/

/*
 * INPUT: buf -- length must be be multiples of 512 bytes
 * RETURN VALUE: 1 if successful, 0 if error
 * */
uint32_t ata_read_pio28(uint8_t* buf, uint32_t sector, uint32_t sec_count) {
	/* Sanity check */
	if (NULL == buf || sector > 0x3FFFFF) {
		return 0;
	}
	/* variable we need to use */
	uint32_t status;
	int32_t i;
	uint16_t data;
	while (sec_count > 0) {
		// printf("read once,sec_count is %d\n",sec_count);
		outb(ATA_RW_MASTER | (ATA_MASTER_SLAVEBIT << 4) | ((sector >> 24) & 0x0F), ATA_DRIVE_SELECT);
		/*ignore the error port to better performance*/
		outb(1, ATA_SECTOR_COUNT);
		// send sector to different port
		outb(sector & ATA_8BIT_MASK, ATA_LBALO);
		outb((sector >> 8) & ATA_8BIT_MASK, ATA_LBAMID);
		outb((sector >> 16) & ATA_8BIT_MASK, ATA_LBAHI);
		// send read sectors command
		outb(ATA_CMD_READ, ATA_STATUS);
		//wait for BSY clear and DRQ set
		while (1) {
			status = inb(ATA_STATUS);
			if (!(status & ATA_BSY_MASK) && (status & ATA_DRQ_MASK)) {
				//read to buffer
				for (i = 0; i <ATA_SECTOR_SIZE; i = i + 2 ){
					// the ATA DATA port send 16 bit every time
					data = inw(ATA_DATA);
					buf[i+1] = (data >> 8) & ATA_8BIT_MASK;
					buf[i] = data & ATA_8BIT_MASK;
				}
				// update the buf, sector and counter
				sector++;
				sec_count--;
				buf += ATA_SECTOR_SIZE;
				break;
			}
		}
	}
	return 1;
}

/*
 * INPUT: buf -- length must be be multiples of 512 bytes
 * */
uint32_t ata_write_pio28(uint8_t* buf, uint32_t sector, uint32_t sec_count) {
	/* Sanity check */
	if (NULL == buf || sector > 0xFFFFFFF) {
		return 0;
	}
	/* variable we need to use*/
	uint32_t i;
	uint32_t write_sec_num = 0;
	// uint32_t status;
	uint16_t data;
	
	while (sec_count > write_sec_num) { 
		// nearly the same with read, except for the command line
		outb(ATA_RW_MASTER | (ATA_MASTER_SLAVEBIT << 4) | (((write_sec_num + sector) >> 24) & 0xF), ATA_DRIVE_SELECT);
		outb(1, ATA_SECTOR_COUNT);

		outb((write_sec_num + sector) & ATA_8BIT_MASK, ATA_LBALO);
		outb(((write_sec_num + sector) >> 8) & ATA_8BIT_MASK, ATA_LBAMID);
		outb(((write_sec_num + sector) >> 16) & ATA_8BIT_MASK, ATA_LBAHI);

		outb(ATA_CMD_WRITE, ATA_STATUS);
		// wait until DRQ set
		while(!(inb(ATA_STATUS) & ATA_DRQ_MASK)) {}

		for (i = 0; i < ATA_SECTOR_SIZE; i = i + 2) {
			/* write the data */
			data = buf[i + ATA_SECTOR_SIZE * write_sec_num] + (buf[i+1 + ATA_SECTOR_SIZE * write_sec_num] << 8);
			outw(data, ATA_DATA);
			/* Cache Flush */
			outb(ATA_MASTER_DRIVE, ATA_DRIVE_SELECT);
			outb(ATA_CMD_CACHE_FLUSH, ATA_STATUS);
			// wait BSY clear
			while (inb(ATA_STATUS) & ATA_BSY_MASK) {}
		}
		write_sec_num++;
	}
	return 1;
}
