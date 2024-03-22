#ifndef _ATA_H
#define _ATA_H

#include "types.h"

// I/O ports
#define ATA_DATA			0x1F0
/*we don't use error port so I don't define it here*/
#define ATA_SECTOR_COUNT	0x1F2
#define ATA_LBALO			0x1F3
#define ATA_LBAMID			0x1F4
#define ATA_LBAHI   		0x1F5
#define ATA_DRIVE_SELECT	0x1F6
#define ATA_STATUS			0x1F7

#define ATA_MASTER_DRIVE	0xA0
#define ATA_SLAVE_DRIVE		0xB0

#define ATA_BSY_MASK		0x80
#define ATA_DRQ_MASK		0x08
#define ATA_ERR_MASK		0x01
#define ATA_DF_MASK			0x20
#define ATA_8BIT_MASK       0xFF

#define ATA_MASTER_SLAVEBIT	0x0
#define ATA_SLAVE_SLAVEBIT	0x1
#define ATA_RW_MASTER		0xE0
#define ATA_RW_SLAVE		0xF0

#define ATA_CMD_READ		0x20
#define ATA_CMD_WRITE		0x30
#define ATA_CMD_CACHE_FLUSH	0xE7

#define ATA_SECTOR_SIZE		512

uint32_t ata_read_pio28(uint8_t* buf,uint32_t sector, uint32_t sec_count);
uint32_t ata_write_pio28(uint8_t* buf,uint32_t sector, uint32_t sec_count);

#endif /* _ATA_H */
