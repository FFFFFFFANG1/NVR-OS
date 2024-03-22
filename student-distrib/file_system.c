#include "file_system.h"
#include "systemcall.h"
#include "ata.h"

// file variable
boot_block_t* boot_block; // Pointer to the boot block
inode_t* inode_block;  // Pointer to the inodes
data_block_t* data_block;  // Pointer to the data blocks
dentry_t open_dir; // dentry for the directory read
dentry_t open_file; // dentry for the file read
uint32_t dir_index;
// 63 for the max inde num
uint8_t inode_block_bitmask[63];
// 63 for the max dentry num
// uint8_t dentry_bitmask[63];
// 63 for the max file num and 1023 for each file's max data block num
uint8_t data_block_bitmask[63 * 1023];
// number of data block that has been used
uint32_t data_block_used;
/*
read_dentry_by_name
    DESCRIPTION: read directory entry according to the name 
    INPUT: 
    fname: file name we want to read
    dentry: pointer to the reading dentry
    OUTPUT: return 0 if success and -1 if fail
    SIDE EFFECT: write the dentry
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    // declare the variable we need to use
    int i;
    uint8_t* file_name;
    //sanity check: invalid file name (too long) or NULL pointer
    if (strlen( (int8_t*)fname ) > FILENAME_LEN || fname == NULL){
        printf("invalid argument for read_dentry_by_name\n");
        return -1;
    }

    // loop in the directory entries to find the file
    for (i = 0; i < boot_block->dir_count; i++){
        file_name = boot_block->dir_entries[i].filename;
        if (strncmp((int8_t*)fname, (int8_t*)file_name, FILENAME_LEN) == 0){ // means we find the file that has the same name
            // copy to the dentry: file name; file type; inode number
            strncpy((int8_t*)(dentry->filename), (int8_t*)(fname), FILENAME_LEN);
            dentry->filetype = boot_block->dir_entries[i].filetype;
            dentry->inode_num = boot_block->dir_entries[i].inode_num;
            return 0;
        }
    }
    // loop end, we don't find so the file don't exist, fail
    return -1;
}

/*
read_dentry_by_index
    DESCRIPTION: read directory entry according to the index
    INPUT:
    index: dentry index we want to read
    dentry: pointer to the reading dentry
    OUTPUT: return 0 if success and -1 if fail
    SIDE EFFECT: write the dentry
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    // declare the variable we need to use
    uint8_t* fname;

    //sanity check: invalid index
    if (index >= boot_block->dir_count || index >= FILENUM_MAX || index < 0){
        return -1;
    }

    fname = boot_block->dir_entries[index].filename;
    // copy to the dentry: file name; file type; inode number
    strncpy((int8_t*)(dentry->filename), (int8_t*)(fname), FILENAME_LEN);
    dentry->filetype = boot_block->dir_entries[index].filetype;
    dentry->inode_num = boot_block->dir_entries[index].inode_num;
    return 0;

}

/*
read_data
    DESCRIPTION: read the data of fixed length from the given inode and offset
    INPUT:
    inode: the inode we want to read
    offset: the offset in the file
    buf: get the bytes we read
    length: the length of byte we are going to read
    OUTPUT: the number of bytes we read
    SIDE EFFECT: change data in the buf
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    // the variable we need to use
    uint32_t i;
    uint32_t byte_counter;
    uint32_t start_index; // data block index we start to read
    uint32_t start_offset; // offset when we start to read
    uint32_t end_index; // data block we end reading
    uint32_t end_offset; // offset when we end reading
 
    uint32_t cur_index; // current data block index
    data_block_t* cur_block; // Point to current data block

    // sanity check
    if(inode >= boot_block->inode_count - 1 || offset > inode_block[inode].len || length == 0){
        return 0;
    }

    if (length + offset > inode_block[inode].len){
        length = inode_block[inode].len - offset;
    }

    byte_counter = 0;

    // get the start (end) index and offset
    start_index = offset / BLOCK_SIZE;
    end_index = (offset + length - 1) / BLOCK_SIZE;
    start_offset = offset % BLOCK_SIZE;
    end_offset = (offset + length - 1) % BLOCK_SIZE;

    // only one data block condition
    if(start_index == end_index){
        // get the index and block we want to read
        cur_index = (inode_block[inode].data_block_num)[start_index];
        cur_block = &(data_block[cur_index]);
        // copy "length" byte
        memcpy(buf, &(cur_block->data[start_offset]), length);
        byte_counter += length;
        return byte_counter;
    }

    for(i = start_index; i <= end_index; i++){
        // get the index and block we want to read
        cur_index = (inode_block[inode].data_block_num)[i];
        cur_block = &(data_block[cur_index]);
        // start block
        if(i == start_index){
            // start from start offset
            memcpy(buf, &(cur_block->data[start_offset]), BLOCK_SIZE - start_offset);
            byte_counter += BLOCK_SIZE-start_offset;
        }
        // end block
        else if(i == end_index){
            memcpy(buf + byte_counter, cur_block->data, end_offset + 1 );
            byte_counter += end_offset + 1;
            return byte_counter;
        }
        // else, all the data (4kB) should be copied
        else {
            memcpy(buf + byte_counter, cur_block->data, BLOCK_SIZE);
            byte_counter += BLOCK_SIZE;
        }    
    }
    return byte_counter;
}

/*
file_open
    DESCRIPTION: open the file
    INPUT:
    file_name: the file name we want to open
    OUTPUT: return 0 if success and -1 if fail
    SIDE EFFECT: none
*/
int32_t file_open (const uint8_t* file_name){
    // the variable we need to use
    int32_t rev;
    dentry_t dentry;
    // open the file
    rev = read_dentry_by_name(file_name,&(dentry));
    return rev;
}

/*
file_close
    DESCRIPTION: close the file
    INPUT: fd(file descriptor)
    OUTPUT: 0 if success (always success)
    SIDE EFFECT: close the file
*/
int32_t file_close (int32_t fd){
    return 0;
}

/*
file_read
    DESCRIPTION: read the file based on the file descriptor
    INPUT:
    fd: file descriptor
    buf: get the byte we read
    nbytes: the lengh of byte we want to read
    OUTPUT: return the number of bytes we read
    SIDE EFFECT: read the files
*/
int32_t file_read (int32_t fd, void* buf, int32_t nbytes){
    // the variable we need to use
    int32_t cur_pid;
    // sanity check
    if (buf == 0) {
        return 0;
    }
    // get the cur_pcb and find the inode, offset in the file_decs[fd]
    int32_t esp;
    asm volatile ("          \n\
                 movl %%esp, %0  \n\
            "
            :"=r"(esp)
            );
    cur_pid = (BAMB - esp) / BAKB;
    pcb_t* cur_pcb = get_pcb(cur_pid);
    // read the data
    return read_data(cur_pcb->file_decs[fd].inode, cur_pcb->file_decs[fd].file_position, (uint8_t *) buf, nbytes);
}

/*
directory_open
    DESCRIPTION: open the directory
    INPUT:
    file_name: the file name we want to open
    OUTPUT: return 0 if success and -1 if fail
    SIDE EFFECT: none
*/
int32_t directory_open(const uint8_t* file_name){
    // the variable we need to use
    int32_t rev;
    // open the directory
    rev = read_dentry_by_name(file_name, &(open_dir));
    // check whether it is a directory
    // if (open_dir.filetype != 1){
    //     rev = -1;
    // }
    return rev;
}

/*
directory_close
    DESCRIPTION: close the directory
    INPUT: fd(file descriptor)
    OUTPUT: 0 if success (always success)
    SIDE EFFECT: close the file
*/
int32_t directory_close(int32_t fd){
    return 0;
}

/*
directory_read
    DESCRIPTION: read the directory
    INPUT:
    fd: file descriptor
    buf: get the directory we read
    nbytes: the byte we want to read
    OUTPUT: return the number of bytes we read or 0 if fail
    SIDE EFFECT: cahnge the file range variable
*/
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
    // the variable we need to use
    int rev;
    dentry_t dir_entry;

    // sanity check
    if (buf == NULL || dir_index >= boot_block->dir_count){
        dir_index = 0; //reset the dir_index
        return 0;
    }

    // get the directory we want and go to next index
    dir_entry = boot_block->dir_entries[dir_index];
    dir_index ++;

    // file_size = (inode_block + dir_entry.inode_num)->len;
    memcpy(buf,&dir_entry.filename,FILENAME_LEN);

    // printf("open file is:");
    // for (i = 0; i < strlen((int8_t*)dir_entry.filename); i++){
    //     printf("%c",dir_entry.filename[i]);
    // }
    // printf("\n");
    // get the length of file name
    rev = strlen((int8_t*)dir_entry.filename);
    if (strlen((int8_t*)dir_entry.filename) >= FILENAME_LEN){
        rev = FILENAME_LEN;
    }
    // dir_index = 0;
    return rev;
}

/*
directory_write
    DESCRIPTION: write to the directory (not used since it is read-only file system)
    INPUT: ignore
    OUTPUT: return -1 for the failure (always fail)
    SIDE EFFECT: none
*/
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/****extra credit****/

/*
file_system_init
    DESCRIPTION: initialize the file system
    INPUT:
    file_start_addr: the start address of the file system
    OUTPUT: none
    SIDE EFFECT: change the file variable
*/
void file_system_init(uint32_t file_start_addr){
    /* variable we need to use*/
    uint32_t i,j;
    uint8_t buf[ATA_SECTOR_SIZE];
    uint32_t length;
    dentry_t dentry;
    // read from ata driver (disk) if we write before
    // ata_read_pio28(buf, FILESYS_DISK_FLAG_POS, 1);
    if (buf[0] == FILESYS_DISK_FLAG) {
        // read bootblock to decide the length
        ata_read_pio28((uint8_t*)(boot_block), FILESYS_DISK_POS, SECTOR_PER_BLOCK);
        // length = boot_block->inode_count + boot_block->data_count + 1;
        length = 150;
        // printf("read the boot block from the disk; length is %d",length);
        // load file system
        ata_read_pio28((uint8_t*)file_start_addr, FILESYS_DISK_POS, length * SECTOR_PER_BLOCK);
    }

    // init the file range variable: pointer to boot block/ inode block/ data block
    boot_block = (boot_block_t*)file_start_addr;
    // printf("has %d inode\n",boot_block->inode_count);
    inode_block = (inode_t*)(boot_block + 1);
    data_block = (data_block_t*)(inode_block + boot_block->inode_count);
    dir_index = 0;
    // dentry mask and inode mask
    for (i = 0; i < 63; i++){
        // dentry_bitmask[i] = 0;
        inode_block_bitmask[i] = 0;
    }
    for (i = 0; i < boot_block->dir_count; i++){
        // dentry_bitmask[i] = 1;
        dentry = boot_block->dir_entries[i];
        inode_block_bitmask[dentry.inode_num] = 1;
        // printf("inode number is %d\n",dentry.inode_num);
    }
    // data mask
    data_block_used = 0;
    for (i = 0; i < 63 * 1023; i++){
        data_block_bitmask[i] = 0;
    }
    for (i = 0; i < boot_block->inode_count; i ++){
        if (inode_block_bitmask[i] == 1){
            length = inode_block[i].len / BLOCK_SIZE + 1;
            data_block_used += length;
            for (j = 0; j < length; j++){
                data_block_bitmask[i * 1023 + j] = 1;
            }
        }
        
    }
}

/*
file_write
    DESCRIPTION: write to the file (not used since it is read-only file system)
    INPUT: ignore
    OUTPUT: return -1 for the failure (always fail)
    SIDE EFFECT: none
*/
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes){
    // the variable we need to use
    // uint32_t i;
    int32_t cur_pid;
    pcb_t* cur_pcb;
    file_decs_t* file_desc;
    uint32_t inode;
    uint32_t offset;
    uint32_t byte_counter;
    uint32_t length;
    uint8_t flag_buf[ATA_SECTOR_SIZE];
    /* sanity check */ 
    if (fd < 0 || fd > 7 || buf == NULL || nbytes < 0) {
        return -1;
    }

    /* create the new file */

    /* get the cur_pcb and save the value we need */
    int32_t esp;
    asm volatile ("          \n\
                 movl %%esp, %0  \n\
            "
            :"=r"(esp)
            );
    cur_pid = (BAMB - esp) / BAKB;
    cur_pcb = get_pcb(cur_pid);
    file_desc = cur_pcb->file_decs;
    inode = file_desc[fd].inode;
    offset = inode_block[inode].len;
    /*write the data to the file*/
    byte_counter = write_data(inode, offset, (uint8_t*)buf, nbytes);
    length = boot_block->inode_count + boot_block->data_count + 1; 
    /* save to disk*/
    flag_buf[0] = FILESYS_DISK_FLAG;
    // printf("going to save to disk\n");
	ata_write_pio28(flag_buf, FILESYS_DISK_FLAG_POS, 1);
	ata_write_pio28((uint8_t*)boot_block, FILESYS_DISK_POS, length * SECTOR_PER_BLOCK);
    return byte_counter;
}

/*  write_data:
    DESCRIPTION: write the data to the given inode and offset
    INPUT:
    inode: the inode we want to write
    offset: the offset in the file
    buf: the data we write into file
    length: the length of byte we are going to read
    OUTPUT: the number of bytes we read
    SIDE EFFECT: change data in the buf

*/
int32_t write_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    // the variable we need to use
    uint32_t i;
    uint32_t byte_counter = 0;
    uint32_t start_index; // data block index we start to write
    uint32_t start_offset; // offset when we start to read
 
    uint32_t cur_index; // current data block index
    data_block_t* cur_block; // Point to current data block

    // sanity check
    if(inode >= boot_block->inode_count - 1 || length == 0){
        return 0;
    }
    /*write data to the block*/
    start_index = offset / BLOCK_SIZE;
    start_offset = offset % BLOCK_SIZE;
    // write into current data block is enough
    if (start_offset + length < BLOCK_SIZE){
        cur_index = inode_block[inode].data_block_num[start_index];
        cur_block = &data_block[cur_index];
        memcpy((uint8_t*)cur_block + start_offset, buf, length);
        /*update the file variable*/
        inode_block[inode].len += length;

        byte_counter += length;
        return byte_counter;
    }
    else {
        // we only need to find another data block since we can only input 128 characters once
        for (i = 0; i < 63 * 1023; i++){
            if (data_block_bitmask[i] == 0){
                data_block_bitmask[i] = 1;
                inode_block[inode].data_block_num[start_index + 1] = i;
                break;
            }
        }
        // write into first data block
        cur_index = inode_block[inode].data_block_num[start_index];
        cur_block = &data_block[cur_index];
        byte_counter = BLOCK_SIZE - start_offset;
        memcpy((uint8_t*)cur_block + start_offset, buf, byte_counter);
        // write into second data block
        cur_index = inode_block[inode].data_block_num[start_index + 1];
        cur_block = &data_block[cur_index];
        memcpy((uint8_t*)cur_block, buf + (uint8_t)byte_counter, length - byte_counter);
        byte_counter = length;
        /*update the file variable*/
        inode_block[inode].len += length;

        return byte_counter;
    }
}

/*
    return value: 0 if success and -1 if fail
*/
int32_t file_create(uint8_t* buf){
    printf("create file ing\n");
    // variable we need to use
    int32_t i;
    uint8_t flag_buf[ATA_SECTOR_SIZE];
    int32_t length;
    int32_t inode;
    /*sanity check*/
    if (buf == NULL){
        return -1;
    }
    /*find inode that are not used*/
    for (i = 1; i < boot_block->inode_count; i++){
        if (inode_block_bitmask[i] == 0){
            inode = i;
            printf("get new inode:%d\n",inode);
            break;
        }
    }
    inode_block_bitmask[inode] = 1;
    /*create the file and update*/
    // boot block
    boot_block->dir_entries[boot_block->dir_count].filetype = FILETYPE_REG;
    boot_block->dir_entries[boot_block->dir_count].inode_num = inode;
    for (i = 0; i < FILENAME_LEN; i++){
        boot_block->dir_entries[boot_block->dir_count].filename[i] = buf[i];
    }
    // inode block
    inode_block[inode].len = 0;
    for (i = 0; i < 63 * 1023; i++){
        if (data_block_bitmask[i] == 0){
            data_block_bitmask[i] = 1;
            inode_block[inode].data_block_num[0] = i;
            break;
        }
    }
    boot_block->dir_count ++;
    // file range variable
    dir_index = 0;
    /*save to disk*/
    flag_buf[0] = FILESYS_DISK_FLAG;
    // printf("going to save to disk\n");
	ata_write_pio28(flag_buf, FILESYS_DISK_FLAG_POS, 1);
    length = boot_block->data_count + boot_block->inode_count + 1;
	ata_write_pio28((uint8_t*)boot_block, FILESYS_DISK_POS, length * SECTOR_PER_BLOCK);
    return 0;
}

/*
    return value: 0 if success and -1 if fail
*/
int32_t file_delete(uint8_t* buf){
    // variable we need to use
    uint8_t flag_buf[ATA_SECTOR_SIZE];
    uint8_t clear_buf[BLOCK_SIZE] = { 0 };
    dentry_t dentry;
    uint32_t dentry_index;
    uint32_t length;
    uint32_t i,j;
    int32_t ret;
    uint32_t inode;
    uint8_t* file_name;
    uint32_t data_block_num;
    uint32_t data_block_count;
    data_block_t* cur_block;
    // get the dentry of the file
    ret = read_dentry_by_name(buf,&dentry);
    if (ret == -1){return -1;}
    for (i = 0; i < boot_block->dir_count; i++){
        file_name = boot_block->dir_entries[i].filename;
        if (strncmp((int8_t*)buf, (int8_t*)file_name, FILENAME_LEN) == 0){
            dentry_index = i;
            break;
        }
    }
    // dentry
    for (i = dentry_index; i < boot_block->dir_count - 1; i++){
        for (j = 0; j < FILENAME_LEN; j++){
            boot_block->dir_entries[i].filename[j] = boot_block->dir_entries[i + 1].filename[j];
        }
        boot_block->dir_entries[i].filetype = boot_block->dir_entries[i + 1].filetype;
        boot_block->dir_entries[i].inode_num = boot_block->dir_entries[i + 1].inode_num;
    }
    for (j = 0; j < FILENAME_LEN; j++){
        boot_block->dir_entries[boot_block->dir_count - 1].filename[j] = 0;
    }
    boot_block->dir_count --;
    // inode
    inode = dentry.inode_num;
    inode_block_bitmask[inode] = 0;
    data_block_count = inode_block[inode].len / BLOCK_SIZE + 1;
    inode_block[inode].len = 0;
    // data block
    for (i = 0; i < data_block_count; i++){
        data_block_num = inode_block[inode].data_block_num[i];
        cur_block = &data_block[data_block_num];
        memcpy((uint8_t*)cur_block, clear_buf, BLOCK_SIZE);
    }
    // file range variable
    dir_index = 0;

    /*save to disk*/
    flag_buf[0] = FILESYS_DISK_FLAG;
    // printf("going to save to disk\n");
	ata_write_pio28(flag_buf, FILESYS_DISK_FLAG_POS, 1);
    length = boot_block->data_count + boot_block->inode_count + 1;
	ata_write_pio28((uint8_t*)boot_block, FILESYS_DISK_POS, length * SECTOR_PER_BLOCK);
    return 0;
}

/*
    return value: 0 if success and -1 if fail
*/
int32_t file_copy(uint8_t* src, uint8_t* dsc){
    // variable we need to use
    uint8_t flag_buf[ATA_SECTOR_SIZE];
    uint8_t clear_buf[BLOCK_SIZE] = { 0 };
    uint8_t data_buf[BLOCK_SIZE];
    dentry_t src_dentry;
    dentry_t dsc_dentry;
    uint32_t i;
    // uint32_t j;
    int32_t ret;
    uint32_t src_inode;
    uint32_t dsc_inode;
    uint32_t length;
    uint32_t data_block_num;
    data_block_t* cur_dsc_block;
    /* sanity check*/
    if (src == NULL || dsc == NULL){
        return -1;
    }
    /* read src and dsc dentry */
    ret = read_dentry_by_name(src, &src_dentry) + read_dentry_by_name(dsc, &dsc_dentry);
    if (ret != 0){return -1;}
    src_inode = src_dentry.inode_num;
    dsc_inode = dsc_dentry.inode_num;
    /* copy the data from src to dsc using file_write */ 
    // calculate how many data block need to copy first
    length = inode_block[src_inode].len / BLOCK_SIZE + 1;
    // update the dsc's inode block
    inode_block[dsc_inode].len = inode_block[src_inode].len;
    // loop every data block
    for (i = 0; i < length; i++){
        if (i == length - 1){
            length = inode_block[src_inode].len % BLOCK_SIZE;
            ret = read_data(src_inode, i * BLOCK_SIZE, data_buf, length);
            // printf("the data is: ");
            // for (j = 0; j < 10; j++){
            //     printf("%c",data_buf[j]);
            // }
            // printf("\n");
            data_block_num = inode_block[dsc_inode].data_block_num[i];
            cur_dsc_block = &data_block[data_block_num];
            memcpy((uint8_t*)cur_dsc_block, clear_buf, BLOCK_SIZE);
            write_data(dsc_inode, i * BLOCK_SIZE, data_buf, length);
            break;
        }
        else {
            ret = read_data(src_inode, i * BLOCK_SIZE, data_buf, BLOCK_SIZE);
            // printf("the data is: ");
            // for (j = 0; j < 10; j++){
            //     printf("%c",data_buf[j]);
            // }
            // printf("\n");
            data_block_num = inode_block[dsc_inode].data_block_num[i];
            cur_dsc_block = &data_block[data_block_num];
            memcpy((uint8_t*)cur_dsc_block, clear_buf, BLOCK_SIZE);
            write_data(dsc_inode, i * BLOCK_SIZE, data_buf, BLOCK_SIZE);
        }
        

    }
    /*save to disk*/
    flag_buf[0] = FILESYS_DISK_FLAG;
    // printf("going to save to disk\n");
	ata_write_pio28(flag_buf, FILESYS_DISK_FLAG_POS, 1);
    length = boot_block->data_count + boot_block->inode_count + 1;
	ata_write_pio28((uint8_t*)boot_block, FILESYS_DISK_POS, length * SECTOR_PER_BLOCK);
    return 0;
}