#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#ifndef ASM

#include "lib.h"
#include "multiboot.h"


// macro for the file system
#define FILENAME_LEN 32
#define FILENUM_MAX 63
#define BLOCK_SIZE 4096 // 4kB 

#define FILETYPE_RTC 0 // a file giving user-level access to the RTC
#define FILETYPE_DIR 1 // a file giving user-level access to the directory
#define FILETYPE_REG 2 // a file giving user-level access to the regular file

#define FILESYS_DISK_POS        1000 // File system is at sector 1 in the hard drive
#define FILESYS_DISK_FLAG_POS   999 // Flag indicating file system exists in hard drive
#define FILESYS_DISK_FLAG       0x13
#define SECTOR_PER_BLOCK        8

// struct we need to use for the file system
// magic number here are from Appendix A, the figure of the file system

typedef struct  dentry_t { 
    uint8_t  filename[FILENAME_LEN]; // up to 32 characters
    uint32_t filetype;
    uint32_t inode_num;
    uint8_t reserved[24];  
}   dentry_t;

typedef struct  boot_block_t {
    uint32_t dir_count; // directory entry number
    uint32_t inode_count; // inode block number
    uint32_t data_count; // data block number
    uint8_t reserved[52]; // 52B reserved
    dentry_t dir_entries[FILENUM_MAX]; // hold up to 63 files (first always ".")
}   boot_block_t;



typedef struct  inode_t {
    uint32_t len; // length in 4B
    // 1023 data_block indices: a block has 4096B, and len takes 4B, each data block take 4B
    uint32_t data_block_num[(BLOCK_SIZE - 4) / 4]; 
}   inode_t;

typedef struct  datablock_t {
    uint8_t data[BLOCK_SIZE]; // each data block has 4kB
}   data_block_t;

// function 

// read the directory entry by file name
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

// read the directory entry by directory index  
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

//read the bytes length of data from the given file 
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

// write the data
int32_t write_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

// file initialize
void file_system_init(uint32_t file_start_addr);

// open file
int32_t file_open (const uint8_t* file_name);

// close file
int32_t file_close (int32_t fd);

// read a specific file 
int32_t file_read (int32_t fd, void* buf, int32_t nbytes);

// write to file, not used
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes);

// open directory
int32_t directory_open(const uint8_t* file_name);

// close directory
int32_t directory_close(int32_t fd);

// read to directory
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);

// write to directory, not used
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);

// file create
int32_t file_create(uint8_t* buf);

// file delete
int32_t file_delete(uint8_t* buf);

// file copy
int32_t file_copy(uint8_t* src, uint8_t* dsc);

#endif /* ASM */
#endif /* _FILESYSTEM_H */
