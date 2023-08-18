
#include "lib.h"
#include "rtc.h"
#include "terminal.h"
#include "types.h"

#define MAX_PID     6
#define USER_CODE   0x8048000
#define EIGHT_MB    0x800000
#define FOUR_MB     0x400000  
#define EIGHT_KB    0x2000
#define ELF0        0x7F
#define ELF1        0x45
#define ELF2        0x4C
#define ELF3        0x46
#define MAX_FD      8
#define MAX_ARG_LEN 1024
#define MAX_CMD_LEN MAX_FILE_NAME_LEN + MAX_ARG_LEN
#define MAX_FILE_NAME_LEN 32
#define HALT_EXC    100
#define EXC_ERR_CODE    256
#define TEMPBUFSIZE 4
#define INSTR_OFF   24
#define USER_ESP    0x8400000 - 4
#define USERMEM     0x8000000
#define SCREEN_START    (uint8_t*)0x84b8000
#define ESP0_OFFSET 4

#ifndef ASM



/* File Operation Table */
typedef struct {    /* Function pointers for each file type to its driver */
    int (*read)  (int32_t fd, int8_t* buf, int32_t nbytes);
    int (*write) (int32_t fd, const int8_t* buf, int32_t nbytes);
    int (*open)  (const int8_t* filename);
    int (*close) (int32_t fd);
 } fot_t;

 /* File Descriptor Entry */
typedef struct {
    fot_t*  fot_ptr;    /* function table for the file                                          */
    int32_t inode;      /* inode of the file                                                    */
    int32_t file_pos;   /* keeps track of where the user is currently reading from in the file  */
    int32_t flag;       /* flag whether file descriptor is in use or not */
} fde_t;

/* Process Control Block */
typedef struct {
    int32_t pid;
    int32_t parent_id;
    fde_t   fdt[MAX_FD];
    int32_t saved_esp;
    int32_t saved_ebp;
    int32_t sch_esp; // for switching tasks
    int32_t sch_ebp;
    int32_t active;
    int8_t  arg[MAX_ARG_LEN]; /* arguments to pass into file */
} pcb_t;

int cur_pid;
pcb_t *  cur_pcb;



void syscall_init(void);
int32_t halt (uint8_t status);
int32_t execute (const int8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);
int32_t haltall (uint8_t status);

extern void flushTLB(void);

/* these are dummy functions*/
int std_read(int32_t fd, char* buf, int32_t nbytes);
int std_write(int32_t fd, const char* buf, int32_t nbytes);

#endif
