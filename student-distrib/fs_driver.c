#include "fs_driver.h"
#include "syscall.h"

extern int cur_pid;
extern pcb_t *  cur_pcb;

/* void fs_init(uint32_t fs_ptr)
 * initializes the File system
 * Inputs: fs_ptr - Pointer to the base of the file system
 * Outputs: None
 * Side Effects: Initializes the file system and sets some global variables
 */
void fs_init(uint32_t fs_ptr) {

    boot_block_t* bootPtr;
    bootPtr = (boot_block_t *) fs_ptr;

    // set global variables with dir, inode, and data count
    g_dir_count = bootPtr->dir_count;
    g_inode_count = bootPtr->inode_count;
    g_data_count = bootPtr->data_count;

    // set global variables for inode and data ptr
    inode_ptr = fs_ptr + BLOCK_SIZE;
    data_ptr = inode_ptr + BLOCK_SIZE * g_inode_count;
}


/* int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
 * Reads a dentry using the name
 * Inputs: fsname - name to search for
           dentry - pointer to dentry object to fill
 * Outputs: int32_t - Number of bytes written
                      -1 if dentry not found
 * Side Effects: Fills a dentry object with data from the file system
 */
int32_t read_dentry_by_name (const int8_t* fname, dentry_t* dentry) {
    
    int i;
    boot_block_t* fs_ptr;
    dentry_t* fs_dentry;

    fs_ptr = (boot_block_t *) boot_block_ptr;

    int len = strlen((int8_t *)fname);
    if (len > MAX_FILE_NAME || len <= 0) return -1;

    // Loop through all dentries
    for (i = 0; i < DENTRY_SIZE; i++) {
        fs_dentry = &(fs_ptr->direntries[i]);
        // strncmp zero means same
        if (strncmp((int8_t *)fname, fs_dentry->filename, MAX_FILE_NAME) == 0) {
            read_dentry_by_index(i, dentry);
            return 0;
        }
    }
    return -1;
}


/* int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
 * Reads a dentry using the index
 * Inputs: index  - index of dentry to search for
           dentry - pointer to dentry object to fill
 * Outputs: int32_t - Number of bytes written
                      -1 if dentry not found
 * Side Effects: Fills a dentry object with data from the file system
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {

    boot_block_t* fs_ptr;
    dentry_t* fs_dentry;

    // if index is greater than dir count then error
    if (index > g_dir_count) {
        return -1;
    }

    fs_ptr = (boot_block_t *) boot_block_ptr;
    fs_dentry = &(fs_ptr->direntries[index]);

    // strncpy (dest, src, # of chars) in lib.h
    // populate dentry
    strncpy(dentry->filename, fs_dentry->filename, MAX_FILE_NAME);
    dentry->filetype = fs_dentry->filetype;
    dentry->inode_num = fs_dentry->inode_num;
    
    return 0;
}

/* int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 * Fills a buffer with data from a specific inode
 * Inputs: inode  - index of inode to read from
           offset - offset into inode to start reading from
           buf    - buffer to fill with read data
           length - length of data to read into buffer
 * Outputs: int32_t - Number of bytes written
                      -1 if inode not found
 * Side Effects: Fills a buffer with data from an inode
 */
int32_t read_data (int32_t inode, uint32_t offset, int8_t* buf, uint32_t length) {

    // calculate pointer to inode
    inode_t * cur_inode = (inode_t *)(inode_ptr + inode * BLOCK_SIZE);

    // inode bound check
    if ((inode >= g_inode_count) || (inode < 0)) {
        return -1;
    }

    int ilen = cur_inode->length;

    if (offset >= ilen)
    {
        return 0;
    }

    int i;

    int db = offset / BLOCK_SIZE;   // block idx
    int dbidx = offset % BLOCK_SIZE; //byte ptr

    // uint8_t charbuf[length];

    // loop through each byte
    for (i = 0; i < length; i++)
    {
        // eof reached
        if (i + offset >= ilen)
        {
            break;
        }
        
        // bound check
        if (cur_inode->data_block_num[db] >= g_data_count)
        {
            return -1;
        }

        // caluclate ptr to current byte
        uint8_t * cur_data_ptr = (uint8_t *)(data_ptr + BLOCK_SIZE * (cur_inode->data_block_num[db]) + dbidx);

        memcpy(buf+i, cur_data_ptr, 1);

        dbidx++; // increment byte ptr

        // if current data block read, reset byte pointer to 0 and increment data block idx
        if(dbidx == BLOCK_SIZE)
        {
            dbidx = 0;
            db++;
        }
    }
    return i;
}

/* int32_t file_open (const uint8_t* fname)
 * If an FDE is available, sets the corresponding fields for fde and returns fd
 * Inputs: fname: name of file to be opened
 * Outputs: int32_t - fd index
                      -1 if no fde available
 * Side Effects: Modifies the fdt
 */
int32_t file_open (const int8_t* fname)
{
    return 0;
}

/* int32_t file_close (int32_t fd)
 * Closes file by deleting fde and setting flag to 0
 * Inputs: fd: file descriptor
 * Outputs: int32_t - 0 if success
 *                    -1 if fail
 * Side Effects: Modifies the fdt
 */
int32_t file_close (int32_t fd)
{
    return 0;
}

/* file_write (int32_t fd, uint8_t* buf, uint32_t length)
 * Does nothing since read only file system
 * Inputs: fd: file descriptor
 *         buf: Buffer with data to be written
 *         len: length: bytes to be written
 * Outputs: int32_t - 0 if success
 *                    -1 if fail
 * Side Effects: Does nothing
 */
int32_t file_write (int32_t fd, const int8_t* buf, int32_t nbytes)
{
    return -1;
}

/* int32_t file_read (int32_t fd, uint8_t* buf, uint32_t length)
 * Reads 'length' bytes of data from file based on fd and updates buffer
 * Inputs: fd: file descriptor
 *         buf: Buffer with data thats been read
 *         len: length: bytes to be read
 * Outputs: int32_t - number of bytes suceesfully read
 *                    -1 if fail
 * Side Effects: Updates fdt and buffer
 */
int32_t file_read (int32_t fd, int8_t* buf, int32_t nbytes)
{

    // bound and edge tests
    if ((buf == NULL) || (cur_pcb->fdt[fd].flag == 0) || (fd >= MAX_FD) || (fd < 0))
    {
        return -1;
    }

    // call read data with inode from fdt and offset at file_pos
    int32_t read_bytes = read_data (cur_pcb->fdt[fd].inode, cur_pcb->fdt[fd].file_pos, buf, nbytes);

    if (read_bytes == -1)
    {
        return -1;
    }

    // update file_pos nased on no. of bytes read
    cur_pcb->fdt[fd].file_pos += read_bytes;
    return read_bytes;
}

/* int32_t dir_open (const uint8_t* fname)
 * If an FDE is available, sets the corresponding fields for fde and returns fd
 * Inputs: fname: opens root dir
 * Outputs: int32_t - fname
                      -1 if no fde available
 * Side Effects: Modifies the fdt
 */
int32_t dir_open (const int8_t* fname)
{
    return 0;
}

/* int32_t dir_close (int32_t fd)
 * Closes dir by deleting fde and setting flag to 0
 * Inputs: fd: file descriptor
 * Outputs: int32_t - 0 if success
 *                    -1 if fail
 * Side Effects: Modifies the fdt
 */
int32_t dir_close (int32_t fd)
{
    return 0;
}

/* int32_t dir_read (int32_t fd, uint8_t* buf, uint32_t length)
 * Reads 'length' bytes of data from file based on fd and updates buffer
 * Inputs: fd: file descriptor
 *         buf: Buffer with data thats been read
 *         len: length: bytes to be read
 * Outputs: int32_t - number of bytes suceesfully read
 *                    -1 if fail
 * Side Effects: Updates fdt and buffer
 */
int32_t dir_read (int32_t fd, int8_t* buf, int32_t nbytes)
{
    // edge checks
    if ((buf == NULL) || (cur_pcb->fdt[fd].flag == 0) || (fd >= MAX_FD) || (fd < 0))
    {
        return -1;
    }

    // icnrement file pos to point to next file
    int32_t cur_pos = cur_pcb->fdt[fd].file_pos;
    cur_pcb->fdt[fd].file_pos++;

    dentry_t d;

    if (read_dentry_by_index(cur_pos, &d) == -1)
    {
        return -1;
    }

    // copy curr file name into buffer
    strncpy((int8_t *)buf, d.filename, MAX_FILE_NAME);

    // max length should be 32
    int len = strlen(d.filename);
    if (len > MAX_FILE_NAME) len = MAX_FILE_NAME;

    return len;

}

/* dir_write (int32_t fd, uint8_t* buf, uint32_t length)
 * Does nothing since read only file system
 * Inputs: fd: file descriptor
 *         buf: Buffer with data to be written
 *         len: length: bytes to be written
 * Outputs: int32_t - 0 if success
 *                    -1 if fail
 * Side Effects: Does nothing
 */
int32_t dir_write (int32_t fd, const int8_t* buf, int32_t nbytes)
{
    return -1;
}
