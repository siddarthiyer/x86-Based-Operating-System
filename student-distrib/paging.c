#include "paging.h"


// int32_t buf[100000] __attribute__((aligned (PAGESIZE)));

/*
*   void paging_init(void)
*   initializes paging 
*   args: none
*   ret: void
*/
void paging_init(void) {

    int i;

    /*sets up page table for PD[0]
      Present set to 1 for VIDMEM idx and 0 otherwise */
    for (i = 0; i < TABLESIZE; i++)
    {
        page_table[i].read_write = 1;
        page_table[i].user_supervisor = 0; 
        page_table[i].write_through = 0;
        page_table[i].cache_disable = 0;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].page_attribute_table = 0;
        page_table[i].global = 0;
        page_table[i].available_3 = 0;
        page_table[i].address_31_12 = i;
        
        if (i >= 0xb8 && i<= 0xbb) // index b8 - bb are vidmem, and terminal back up pages
        {
            page_table[i].user_supervisor = 1;
            page_table[i].present = 1;
        }
        else if (i >= 0xA0 && i<(0xA0 + 75))
        {
            page_table[i].user_supervisor = 1;
            page_table[i].present = 1;
        }
        else
        {
            page_table[i].present = 0;
        }
    }

    for (i = 0; i < TABLESIZE; i++)
    {
        vidmem_table[i].read_write = 1;
        vidmem_table[i].user_supervisor = 0; 
        vidmem_table[i].write_through = 0;
        vidmem_table[i].cache_disable = 0;
        vidmem_table[i].accessed = 0;
        vidmem_table[i].dirty = 0;
        vidmem_table[i].page_attribute_table = 0;
        vidmem_table[i].global = 0;
        vidmem_table[i].available_3 = 0;
        vidmem_table[i].address_31_12 = i;
        
        if (i ==(VIDMEM >> ADDRSHIFT))
        {
            vidmem_table[i].user_supervisor = 1;
            vidmem_table[i].address_31_12 = VIDMEM >> ADDRSHIFT;
            vidmem_table[i].present = 1;
        }
        else
        {
            vidmem_table[i].present = 0;
        }
    }
    


    /*sets up page directory for PD[0] */
    for (i = 0; i < DIRSIZE; i++)
    {
        page_directory[i].read_write = 1;
        page_directory[i].write_through = 0;
        page_directory[i].cache_disable = 0;
        page_directory[i].accessed = 0;
        page_directory[i].available_1 = 0;
        page_directory[i].available_3 = 0;
        page_directory[i].global = 0;

        /* index 0: 0-0x400000 -> video mem*/
        if (i == 0)
        {
            page_directory[i].page_size = 0;
            page_directory[i].address_31_12 = (uint32_t)(page_table) >> ADDRSHIFT;
            page_directory[i].user_supervisor = 0;
            page_directory[i].present = 1;
        }

        /* index 1: 0x400000-0x800000 -> Kernel Mem*/
        else if (i == 1)
        {
            page_directory[i].page_size = 1;
            page_directory[i].address_31_12 = KERNEL >> ADDRSHIFT;
            page_directory[i].user_supervisor = 0;
            page_directory[i].present = 1; 
        }

        else if (i == USERIDX)
        {
            page_directory[i].page_size = 1;
            page_directory[i].address_31_12 = USER >> ADDRSHIFT;
            page_directory[i].user_supervisor = 1;
            page_directory[i].present = 1; 
        }
        else if (i == VGAIDX)
        {
            page_directory[i].page_size = 1;
            page_directory[i].address_31_12 = VGAADDR >> ADDRSHIFT;
            page_directory[i].user_supervisor = 1;
            page_directory[i].present = 1; 
        }
        else if (i == TERM1)
        {
            page_directory[i].page_size = 1;
            page_directory[i].address_31_12 = TERM1ADDR >> ADDRSHIFT;
            page_directory[i].user_supervisor = 1;
            page_directory[i].present = 1; 
        }
        else if (i == TERM2)
        {
            page_directory[i].page_size = 1;
            page_directory[i].address_31_12 = TERM2ADDR >> ADDRSHIFT;
            page_directory[i].user_supervisor = 1;
            page_directory[i].present = 1; 
        }
        else if (i == TERM3)
        {
            page_directory[i].page_size = 1;
            page_directory[i].address_31_12 = TERM3ADDR >> ADDRSHIFT;
            page_directory[i].user_supervisor = 1;
            page_directory[i].present = 1; 
        }

        else if (i == VIRVIDMEMIDX)
        {
            page_directory[i].page_size = 0;
            page_directory[i].address_31_12 = (uint32_t)(vidmem_table) >> ADDRSHIFT;
            page_directory[i].user_supervisor = 1;
            page_directory[i].present = 1; 
        }

        /*default-> set Present to 0*/
        else
        {
            page_directory[i].page_size = 1;
            page_directory[i].user_supervisor = 0;
            page_directory[i].present = 0;
        }
    }


    /* load page directory into cr3 and enable paging*/
    loadPageDirectory(page_directory);
    enablePaging();
}
