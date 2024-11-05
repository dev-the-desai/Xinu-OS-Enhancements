/* testandset.S - test_and_set (for x86) */

    .text
    .globl test_and_set

/*-------------------------------------------------------------------------------
 * test_and_set -  X86 lock helper; the call is test_and_set(uint32 *, uint32)
 *-------------------------------------------------------------------------------
 */

 test_and_set:
        /* Create new stack frame */
        pushl   %ebp                /* Save the old base pointer         */
        movl    %esp, %ebp          /* Save old sp into new base pointer */
        
        /* Main exchange processes */
        movl  12(%ebp), %eax        /* set eax to new_value */
		movl  8(%ebp), %ecx         /* set ecx to ptr */
		xchgl %eax, (%ecx)		    /* exchange the values */

        /* Clean up stack frame */
        movl    %ebp, %esp          /* Restore the stack pointer        */
        popl    %ebp                /* Restore the old base pointer     */

        ret                         /* work done, return                */
