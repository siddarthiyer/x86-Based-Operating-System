/** scheduler.c
 *  Schedule tasks in a round-robin fashion
*/

#include "lib.h"
#include "syscall.h"
#include "terminal.h"
#include "scheduler.h"
#include "paging.h"


int terminals_initialized[3] = {0,0,0}; // 3 terminals
/** task_switch
 * DESCRIPTION: switch the current task to the next pid's task
 * INPUTS: pid - the pid of the task to switch to
 * OUTPUTS: none
 * SIDE EFFECTS: none
*/
void task_switch()
{
    static int idx = -1;

    if (term_flag)
    {
        idx++;
        if(idx > 2)     // idx should go from 0 to 2
        {
            idx = 0;
        }
        int prev_term = cur_term;
        cur_term = idx;

        // get pid and pcb ptrs
        cur_pid = terminalState[cur_term].cur_pid;
        int prev_pid = terminalState[prev_term].cur_pid;
        cur_pcb = (pcb_t *)(EIGHT_MB - ((cur_pid+1) * EIGHT_KB));
        pcb_t * prev_pcb = (pcb_t *)(EIGHT_MB - ((prev_pid+1) * EIGHT_KB));

        // get current esp and ebp
        register uint32_t sch_ebp asm("ebp");
        register uint32_t sch_esp asm("esp");

        // launch shells at start
        if (terminals_initialized[idx] == 0)
        {
            if (idx != 0)
            {
                prev_pcb->sch_esp = sch_esp;
                prev_pcb->sch_ebp = sch_ebp;

            }
            terminals_initialized[idx] = 1;
            execute("shell");  
        }


        // save esp and ebp to resume process next time
        prev_pcb->sch_esp = sch_esp;
        prev_pcb->sch_ebp = sch_ebp;


        // remap paging
        page_directory[USERIDX].address_31_12 = (EIGHT_MB + (cur_pid * FOUR_MB)) >> ADDRSHIFT;
        if (cur_term == vis_term)
        {
            vidmem_table[VIDMEMIDX].address_31_12 = VIDMEM >> ADDRSHIFT;
        }
        else
        {
            vidmem_table[VIDMEMIDX].address_31_12 = (VIDMEM + PAGESIZE*(cur_term+1)) >> ADDRSHIFT;    
        }
        flushTLB();

        // context switch
        tss.esp0 = (EIGHT_MB - ((cur_pid) * EIGHT_KB) - sizeof(int));

        //update esp and ebp with new args

        int newesp = cur_pcb->sch_esp;
        int newebp = cur_pcb->sch_ebp;

        asm volatile("\
            movl %0, %%esp; \
            movl %1, %%ebp;"
            :
            : "r"(newesp), "r"(newebp)
            : "%esp", "%ebp"
            );
    }
    else
    {
        idx = -1;
        terminals_initialized[0] = 0;
        terminals_initialized[1] = 0;
        terminals_initialized[2] = 0;
    }
}
