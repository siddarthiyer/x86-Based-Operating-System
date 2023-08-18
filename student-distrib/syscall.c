#include "x86_desc.h"
#include "syscall.h"
#include "fs_driver.h"
#include "paging.h"
#include "terminal.h"

extern pde_t page_directory[DIRSIZE] __attribute__((aligned (PAGESIZE)));
extern pte_t page_table[TABLESIZE] __attribute__((aligned (PAGESIZE)));

int pid[MAX_PID] = {0, 0, 0, 0, 0, 0};



static fot_t rtc_fot;
static fot_t dir_fot;
static fot_t file_fot;
static fot_t stdin_fot;
static fot_t stdout_fot;


/**
 * halt
 * 
 * DESCRIPTION: system call to terminate a process.
 * INPUTS: status - error checks to make sure no exceptions occured
 * OUTPUT: on an exception, the error code will be returned
 * SIDE EFFECTS: return to the parent program's execute system call
*/
int32_t halt (uint8_t status)
{
    cli();
    uint32_t retval = (uint32_t) status;

    // if halted by exception, return 256
    if (status == HALT_EXC)
    {
        retval = EXC_ERR_CODE;
    }

    // if base shell then relaunch
    if (cur_pid == 0 || cur_pid == 1 || cur_pid == 2)
    {
        // cur_pid = -1;
        pid[cur_pid] = 0;
        sti();
        execute("shell");
    }

    pid[cur_pid] = 0;

    // get parent process
    int parent = cur_pcb->parent_id;
    terminalState[cur_term].cur_pid = parent;
    pcb_t * parent_ptr = (pcb_t *)(EIGHT_MB - ((parent+1) * EIGHT_KB));


    // set tss for parent
    tss.ss0 = KERNEL_DS;
    tss.esp0 = (EIGHT_MB - ((parent) * EIGHT_KB) - ESP0_OFFSET);

    // Paging
    page_directory[USERIDX].address_31_12 = (EIGHT_MB + (parent * FOUR_MB)) >> ADDRSHIFT;
    flushTLB();

    //close fds
    int j;
    for (j = 0; j<MAX_FD; j++)
    {
        if ((j >= 2) && (cur_pcb->fdt[j].flag == 1))    //exlucde stdin and stdout
        {
          (* cur_pcb->fdt[j].fot_ptr->close)(j);
        }
        cur_pcb->fdt[j].fot_ptr = 0;
        cur_pcb->fdt[j].inode = 0;
        cur_pcb->fdt[j].file_pos = 0;
        cur_pcb->fdt[j].flag = 0;
    }
    cur_pcb->active = 0;

    // set parent as active
    uint32_t saved_esp = cur_pcb->saved_esp;
    uint32_t saved_ebp = cur_pcb->saved_ebp;
    cur_pid = parent;
    cur_pcb = parent_ptr;

    asm volatile("\
    movl %0, %%esp; \
    movl %1, %%ebp; \
    movl %2, %%eax; \
    sti;    \
    leave; \
    ret; "
    :
    : "r"(saved_esp), "r"(saved_ebp), "r"(retval)
    );

    return 0;
}



int32_t haltall (uint8_t status)
{
    int i;
    for (i = 5; i >= 0; i--)
    {
        pid[i] = 0;
    }
    terminal_init();
    tss.esp0 = 0x800000;
    
    return 0;
}



/**
 * syscall_init
 * 
 * DESCRIPTION:initializes cur_pid and fot pointers
 * INPUTS: None
 * OUTPUT: None
 * SIDE EFFECTS: Updates cur pid and fot
*/
void syscall_init(void)
{
    cur_pid = -1;

    rtc_fot.read   = &rtc_read;
    rtc_fot.write  = &rtc_write;
    rtc_fot.open   = &rtc_open;
    rtc_fot.close  = &rtc_close;

    dir_fot.read   = &dir_read;
    dir_fot.write  = &dir_write;
    dir_fot.open   = &dir_open;
    dir_fot.close  = &dir_close;

    file_fot.read  = &file_read;
    file_fot.write = &file_write;
    file_fot.open  = &file_open;
    file_fot.close = &file_close;

    stdin_fot.read  = &terminal_read;
    stdin_fot.write = &std_write;
    stdin_fot.open  = &terminal_open;
    stdin_fot.close = &terminal_close;

    stdout_fot.read  = &std_read;
    stdout_fot.write = &terminal_write;
    stdout_fot.open  = &terminal_open;
    stdout_fot.close = &terminal_close;
}

/**
 * execute
 * 
 * DESCRIPTION: system call to execute a user level program
 * INPUTS: command - the program to execute and its arguments all as a string
 * OUTPUTS: none
 * SIDE EFFECTS: a new process will be created for the passed in program
*/

int32_t execute (const int8_t* command)
{   
    cli();
    dentry_t d;
    int32_t  inodenum;
    int8_t   tempbuf[TEMPBUFSIZE];
    int8_t   cmd[MAX_CMD_LEN]    = "\0";
    int8_t   file[MAX_FILE_NAME] = "\0";
    int8_t   arg[MAX_ARG_LEN]    = "\0";
    int8_t   arg_len             =   0;
    int i;

    /* Parse args*/
    ///////////////////////////////////////////////////////////////////////////////////////////////
    int8_t *cmd_ptr_l, *cmd_ptr_r;

    int cmd_len = strlen(command);
    memcpy(cmd, command, cmd_len * sizeof(int8_t));

    // find the file name within the command, and copy it to the file string
    for(cmd_ptr_l = cmd; *cmd_ptr_l == ' '; cmd_ptr_l++);
    for(cmd_ptr_r = cmd_ptr_l; *cmd_ptr_r != ' ' && *cmd_ptr_r != '\0'; cmd_ptr_r++);
    strncpy(file, cmd_ptr_l, (cmd_ptr_r - cmd_ptr_l) * sizeof(int8_t));

    // find the arg name within the command, and copy it to the arg string
    if(cmd_ptr_r != '\0')
    {
        for(cmd_ptr_l = cmd_ptr_r; *cmd_ptr_l == ' '; cmd_ptr_l++);
    }
    if(*cmd_ptr_l != '\0')
    {
        for(cmd_ptr_r = cmd + MAX_CMD_LEN; *(cmd_ptr_r - 1) == ' ' || *(cmd_ptr_r - 1) == '\0'; cmd_ptr_r--);
        arg_len = cmd_ptr_r - cmd_ptr_l;
    } 
    if(arg_len != 0)
    {
        strncpy(arg, cmd_ptr_l, arg_len * sizeof(int8_t));
    }


    /*Check file validity*/
    ///////////////////////////////////////////////////////////////////////////////////////////////

    if (read_dentry_by_name(file, &d) == -1)
    {
        return -1;
    }

    inodenum = d.inode_num;

    if (read_data(inodenum, 0, tempbuf, TEMPBUFSIZE) == -1)
    {
        return -1;
    }


    // compare 4 bytes read with the 4 elf bytes

    if ((tempbuf[0] != ELF0) || (tempbuf[1] != ELF1) || (tempbuf[2] != ELF2) || (tempbuf[3] != ELF3))
    {
        return -1;
    }

    // clear 4 bytes of buffer
    tempbuf[0] = 0;
    tempbuf[1] = 0;
    tempbuf[2] = 0;
    tempbuf[3] = 0;

    if (read_data(inodenum, INSTR_OFF, tempbuf, TEMPBUFSIZE) == -1)
    {
        return -1;
    }

    int32_t * instr_ptr = (int*)tempbuf;

    /*Setup Paging*/
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // get pid

    for (i = 0; i < MAX_PID; i++)
    {
        if (pid[i] == 0)
        {
            pid[i] = 1;
            break;
        }
    }

    if (i == MAX_PID)
    {
        return -1;
    }

    cur_pid = i;
    

    
    page_directory[USERIDX].address_31_12 = (EIGHT_MB + (cur_pid * FOUR_MB)) >> ADDRSHIFT;
    flushTLB();

    /*Load file into mem*/
    ///////////////////////////////////////////////////////////////////////////////////////////////
    inode_t * cur_inode = (inode_t *)(inode_ptr + inodenum * BLOCK_SIZE);
    int len = cur_inode->length;

    read_data(inodenum, 0, (void *)USER_CODE, len);

    /*Create PCB/Open FD*/
    ///////////////////////////////////////////////////////////////////////////////////////////////

    cur_pcb = (pcb_t *)(EIGHT_MB - ((cur_pid+1) * EIGHT_KB));

    // Possible that parent need not be cur - 1??
    cur_pcb->pid = cur_pid;

    if (cur_pid == 0 || cur_pid == 1 || cur_pid == 2)
    {
        cur_pcb->parent_id = -1;
    }

    else
    {
        cur_pcb->parent_id = terminalState[vis_term].cur_pid;
        terminalState[vis_term].cur_pid = cur_pid;
    }
    

    // TODO stdin
    cur_pcb->fdt[0].fot_ptr = &stdin_fot;
    cur_pcb->fdt[0].inode = 0;
    cur_pcb->fdt[0].file_pos = 0;
    cur_pcb->fdt[0].flag = 1;

    // TODO stdout
    cur_pcb->fdt[1].fot_ptr = &stdout_fot;
    cur_pcb->fdt[1].inode = 0;
    cur_pcb->fdt[1].file_pos = 0;
    cur_pcb->fdt[1].flag = 1;

    for (i = 2; i < MAX_FD; i++)    // initialize fd 2-7
    {
        cur_pcb->fdt[i].fot_ptr = 0;
        cur_pcb->fdt[i].inode = 0;
        cur_pcb->fdt[i].file_pos = 0;
        cur_pcb->fdt[i].flag = 0;
    }
    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");

    cur_pcb->saved_esp = saved_esp;
    cur_pcb->saved_ebp = saved_ebp;
    
    cur_pcb->active = 1;

    // clear stale args and set new args in the pcb
    memset(cur_pcb->arg, '\0', MAX_ARG_LEN);
    memcpy(cur_pcb->arg, arg, arg_len);


    /*Prepare for context switch*/
    ///////////////////////////////////////////////////////////////////////////////////////////////

    tss.ss0 = KERNEL_DS;
    // 8MB-8KB (pid )-1 byte,
    tss.esp0 = (EIGHT_MB - ((cur_pid) * EIGHT_KB) - sizeof(int));


    // user level stack
    uint32_t esp_ = USER_ESP;

    int ds = USER_DS;
    int cs = USER_CS;

    sti();

    /*Push IRET context to stack*/
    /*IRET*/

    asm volatile("\
        pushl %0; \
        pushl %1; \
        pushfl; \
        pushl %2; \
        pushl %3; \
        iret;"
        : // No outputs
        : "g"(ds), "g"(esp_), "g"(cs), "g"(*instr_ptr)
        : "%eax"
        );

    /*Return*/
    return 0;
}

/**
 * read
 * 
 * DESCRIPTION: system call to read data
 * INPUTS: fd: fd index to read, buf: stores data read, nbytes: bytes to be read
 * OUTPUT: buf: gets populated with data read, ret: returns bytes read
 * SIDE EFFECTS: updates buffer
*/
int32_t read (int32_t fd, void* buf, int32_t nbytes)
{
    if ((fd >= MAX_FD) || (fd < 0))
    {
        return -1;
    }
    if (buf == NULL || nbytes < 0)
    {
        return -1;
    }


    if (cur_pcb->fdt[fd].flag == 0)
    {
        return -1;
    }
    int ret = (* cur_pcb->fdt[fd].fot_ptr->read)(fd, buf, nbytes);
    return ret;
}

/**
 * write
 * 
 * DESCRIPTION: system call to write data
 * INPUTS: fd: fd index to write, buf: stores data to write, nbytes: bytes to be written
 * OUTPUT: ret: returns bytes read
 * SIDE EFFECTS: file gets written to
*/
int32_t write (int32_t fd, const void* buf, int32_t nbytes)
{
    if ((fd >= MAX_FD) || (fd < 0))
    {
        return -1;
    }
    if (buf == NULL || nbytes < 0)
    {
        return -1;
    }
    if (cur_pcb->fdt[fd].flag == 0)
    {
        return -1;
    }
    return (* cur_pcb->fdt[fd].fot_ptr->write)(fd, buf, nbytes);
}

/**
 * open
 * 
 * DESCRIPTION: system call toopen file
 * INPUTS: filename: name of file to be opened
 * OUTPUT: returns FD
 * SIDE EFFECTS: modifies the current pcb
*/
int32_t open (const uint8_t* filename)
{
    dentry_t file_dentry; // pointer to the dentry corresponding to the file name
    uint8_t  fd;          // first open entry in fdt

    // find the file, check for errors
    if (read_dentry_by_name((char *)filename, &file_dentry) == -1)
    {
        return -1;
    }


    // find an open file descriptor from the pcb
    for(fd = 0; 
       (fd < MAX_FD) && (cur_pcb->fdt[fd].flag == 1);
        fd++);

    if(fd == MAX_FD)
    {
        return -1;
    }

    // signal that the current file descriptor is in use, update its attributes
    cur_pcb->fdt[fd].flag     = 1;
    cur_pcb->fdt[fd].file_pos = 0;
    cur_pcb->fdt[fd].inode    = file_dentry.inode_num;
    switch(file_dentry.filetype)
    {
        case RTC_FILETYPE:
            cur_pcb->fdt[fd].fot_ptr = &rtc_fot;
            break;
        case DIR_FILETYPE:
            cur_pcb->fdt[fd].fot_ptr = &dir_fot;
            break;
        case FILE_FILETYPE:
            cur_pcb->fdt[fd].fot_ptr = &file_fot;
            break;
    }

    // if the open function on the file failed, return error
    if((* cur_pcb->fdt[fd].fot_ptr->open)((char *)filename) == -1)
    {
        return -1;
    }

    return fd;
}

/**
 * close
 * 
 * DESCRIPTION: system call to close file
 * INPUTS: fd: fd of file to be closed
 * OUTPUT: closes file
 * SIDE EFFECTS: modifies pcb
*/
int32_t close (int32_t fd)
{
    if ((fd >= MAX_FD) || (fd < 2)) // cannot close stdin and stdout
    {
        return -1;
    }

    if (cur_pcb->fdt[fd].flag == 0)
    {
        return -1;
    }

    cur_pcb->fdt[fd].flag = 0;
    cur_pcb->fdt[fd].file_pos = 0;
    cur_pcb->fdt[fd].inode = 0;
    return (* cur_pcb->fdt[fd].fot_ptr->close)(fd);
}

/*reads the program's command line arguments into a user-level buffer*/
/** getargs
 * DESCRIPTION: reads the program's command line arguments into a user-level buffer
 * INPUTS:
 *      buf     - the buffer to write to
 *      nbytes  - number of bytes to write
 * OUTPUTS: 0/-1 based on success or error
*/
int32_t getargs (uint8_t* buf, int32_t nbytes)
{
    /** return an error if
     *  - argument length is 0
     *  - argument is longer than buffer
     */

    
    if(nbytes > MAX_ARG_LEN)          return -1;
    if(cur_pcb->arg[0] == '\0')       return -1;
    if(strlen(cur_pcb->arg) > nbytes) return -1;

    memcpy(buf, cur_pcb->arg, nbytes);
    return 0;
}

/**
 * vidmap
 * 
 * DESCRIPTION: system call for vidmap
 * INPUTS: double pointer for where screen starts
 * OUTPUT: n/a
 * SIDE EFFECTS: modifies screen_start
*/
int32_t vidmap (uint8_t** screen_start)
{
    if (((int)(screen_start) < USERMEM) ||((int)(screen_start) >= USERMEM + FOUR_MB))
    {
        return -1;
    }

    *screen_start = SCREEN_START;
    return 0;
}


/** set_handler
 * DESCRIPTION: dummy function
 * INPUTS: neglect
 * OUTPUTS: 0
*/
int32_t set_handler (int32_t signum, void* handler_address)
{
    return 0;
}


/** sigreturn
 * DESCRIPTION: dummy function
 * INPUTS: neglect
 * OUTPUTS: 0
*/
int32_t sigreturn (void)
{
    return 0;
}


/** std_read
 * DESCRIPTION: dummy function
 * INPUTS: neglect
 * OUTPUTS: -1
*/
int std_read(int32_t fd, char* buf, int32_t nbytes)
{
    return -1;
}


/** std_write
 * DESCRIPTION: dummy function
 * INPUTS: neglect
 * OUTPUTS: -1
*/
int std_write(int32_t fd, const char* buf, int32_t nbytes)
{
    return -1;
}
