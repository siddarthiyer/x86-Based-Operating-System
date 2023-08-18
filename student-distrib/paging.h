#include "types.h"
#include "x86_desc.h"

#define PAGESIZE    4096
#define DIRSIZE     1024 
#define TABLESIZE   1024
#define VIDMEM      0xB8000
#define VIDMEMIDX   0xB8
#define ADDRSHIFT   12
#define KERNEL      0x400000
#define USER        0x8000000
#define VIRVIDMEM  0x8400000
#define USERIDX     32 
#define VIRVIDMEMIDX    33
#define VGAIDX  34
#define VGAADDR     0x8800000
#define TERM1  35
#define TERM1ADDR     0x8C00000
#define TERM2  36
#define TERM2ADDR     0x9000000
#define TERM3  37
#define TERM3ADDR     0x9400000

/*struct for page directory entry*/
typedef struct __attribute__((packed)) pde_t {
    uint32_t present                : 1;
    uint32_t read_write             : 1;
    uint32_t user_supervisor        : 1;
    uint32_t write_through          : 1;
    uint32_t cache_disable          : 1;
    uint32_t accessed               : 1;
    uint32_t available_1            : 1;
    uint32_t page_size              : 1;
    uint32_t global                 : 1;
    uint32_t available_3            : 3;    // 3 available bits
    uint32_t address_31_12          : 20;   // top 20 bits of addr   
} pde_t;

/*struct for page table entry*/
typedef struct __attribute__((packed)) pte_t {
    uint32_t present                : 1;
    uint32_t read_write             : 1;
    uint32_t user_supervisor        : 1;
    uint32_t write_through          : 1;
    uint32_t cache_disable          : 1;
    uint32_t accessed               : 1;
    uint32_t dirty                  : 1;
    uint32_t page_attribute_table   : 1;
    uint32_t global                 : 1;
    uint32_t available_3            : 3;    // 3 available bits
    uint32_t address_31_12          : 20;   // top 20 bits of addr    
} pte_t;

// pde_t page_directory[DIRSIZE] __attribute__((aligned (PAGESIZE)));
// pte_t page_table[TABLESIZE] __attribute__((aligned (PAGESIZE)));
pte_t vidmem_table[TABLESIZE] __attribute__((aligned (PAGESIZE)));
pde_t page_directory[DIRSIZE] __attribute__((aligned (PAGESIZE)));
pte_t page_table[TABLESIZE] __attribute__((aligned (PAGESIZE)));
pte_t vidmem_table[TABLESIZE] __attribute__((aligned (PAGESIZE)));


/*initializes paging*/
extern void paging_init(void);

/*declaration for enablePaging function in enablepaging.S*/
extern void enablePaging(void);

/*declaration for loadPageDirectory function in enablepaging.S*/
extern void loadPageDirectory(pde_t * page_directory); 
