.text
.globl loadPageDirectory

/*
*   loadPageDirectory:
*   loads page directory address into cr3
*   Arguments: pde_t * page_directory
*   Return type: void
*/
loadPageDirectory:
    push    %ebp
    mov     %esp, %ebp
    mov     8(%esp), %eax   /*store arg into eax*/
    mov     %eax, %cr3      /*move PD addr into cr3*/
    mov     %ebp, %esp
    pop     %ebp
    ret

.text
.globl enablePaging

/* 
*   enablePaging:
*   enables paging by setting corresponding bits in cr4 and cr0
*   Arguments: none
*   Return type: void
*/
enablePaging:
    push    %ebp
    mov     %esp, %ebp
    mov     %cr4, %eax
    or      $0x00000010, %eax   /*set bit 4 in cr4 to enable 4MB pages*/
    mov     %eax, %cr4
    mov     %cr0, %eax
    or      $0x80000001, %eax   /*set bit 31 and 1 in cr0 to enable paging and put system in protected mode*/
    mov     %eax, %cr0
    mov     %ebp, %esp
    pop     %ebp
    ret

.text
.globl flushTLB

/*flush tlb*/
flushTLB:
    mov     %cr3, %eax
    mov     %eax, %cr3
    ret
