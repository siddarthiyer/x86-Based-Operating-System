// Comment

#include "lib.h"

#define DENTRY_SIZE     64
#define BLOCK_SIZE      4096
#define MAX_FILE_NAME   32

#define RTC_FILETYPE  0
#define DIR_FILETYPE  1 
#define FILE_FILETYPE 2

typedef struct __attribute__((packed)) dentry {
    int8_t filename[MAX_FILE_NAME];
    int32_t filetype;
    int32_t inode_num;
    int8_t reserved[24];    // reserved
} dentry_t;

typedef struct __attribute__((packed)) inode {
    int32_t length;
    int32_t data_block_num[1023];   // 1023 total data blocks
} inode_t;

typedef struct __attribute__((packed)) boot_block {
    int32_t dir_count;
    int32_t inode_count;
    int32_t data_count;
    int8_t reserved[52];        // reserved
    dentry_t direntries[63];    // 63 total dentries
} boot_block_t;


int32_t g_dir_count;
int32_t g_inode_count;
int32_t g_data_count;

uint32_t boot_block_ptr;
uint32_t inode_ptr;
uint32_t data_ptr;

void fs_init (uint32_t fs_ptr);

int32_t read_dentry_by_name (const int8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (int32_t inode, uint32_t offset, int8_t* buf, uint32_t length);
int32_t file_open (const int8_t* fname);
int32_t file_close (int32_t fd);
int32_t file_write (int32_t fd, const int8_t* buf, int32_t nbytes);
int32_t file_read (int32_t fd, int8_t* buf, int32_t nbytes);
int32_t dir_open (const int8_t* fname);
int32_t dir_close (int32_t fd);
int32_t dir_write (int32_t fd, const int8_t* buf, int32_t nbytes);
int32_t dir_read (int32_t fd, int8_t* buf, int32_t nbytes);



