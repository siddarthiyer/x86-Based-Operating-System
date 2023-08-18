# syscall_handler.S - handler for 0x80 syscall. takes arg (idx) in %eax, jump table.
# vim:ts=4 noexpandtab

#define ASM     1

# equal to size of jtable
#define MAX_HANDLER_IDX 10


# void syscall_handler()
# Acts as "Dispatcher", uses %EAX to determine which syscall to call.
# prints "invalid syscall" if called with improper value in EAX.
# arg is parameter of the function - is already positioned by caller
# USES JMP INSTEAD OF CALL TO PRESERVE STACK
#       Inputs: %EAX: CMD, identifier for syscall to call 
#       Outputs: always void!
# Register usage:
#       EAX: Input, used for comparisons for jump
# Doesn't really clobber anything else
.globl syscall_handler
syscall_handler:

        # check for valid syscall number
        cmpl    $MAX_HANDLER_IDX, %eax 
        jg      SYSCALL_INVALID 
        cmpl    $1, %eax
        jl     SYSCALL_INVALID

        # Save all general purpose register + flags to the stack

        pushl %ebp
        pushl %esi
        pushl %edi
        pushl %edx
        pushl %ecx
        pushl %ebx

        # args
        pushl %edx
        pushl %ecx
        pushl %ebx
        call    *jump_table(,%eax,4) # each element in jump table is 4 bytes
        addl $12, %esp  # caller teardown

        popl %ebx
        popl %ecx
        popl %edx
        popl %edi
        popl %esi
        popl %ebp
        iret

SYSCALL_INVALID:
        movl $-1, %eax
        iret
              
# Jump table
jump_table:
.long   0, halt, execute, read, write, open, close, getargs, vidmap
