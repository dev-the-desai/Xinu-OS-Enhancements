# Xinu-OS-Enhancements
Projects related to enhancing XINU's functionality

## Xinu OS Process Management Enhancements
Added cascading process termination and Unix-like fork system call to Xinu's process management system. Implemented process stack duplication with distinct parent-child execution paths while preserving memory address space sharing. This project enhances Xinu's process creation and termination capabilities while maintaining system process integrity.

### Key Implementation Features

1. Cascading Process Termination

  Added distinction between system and user processes
  Implemented selective cascading termination for user processes
  Enhanced process kill functionality to handle parent-child relationships
  
2. Fork System Call Implementation

  Created Unix-like fork() functionality
  Proper handling of process stack duplication
  Implemented return value logic:

Returns child PID to parent
Returns NPROC to child process
Returns SYSERR on failure

## Xinu OS Scheduler Modifications
Modified the priority-based scheduler in Xinu to implement Lottery Scheduling and Multi-Level Feedback Queue (MLFQ) scheduling algorithms. This project extends Xinu's process management capabilities while maintaining system process scheduling integrity.

## Xinu OS Synchronization Extensions
Added synchronization locks (spinlocks) to Xinu and implemented advanced locking mechanisms including bounded wait guards, priority inversion avoidance, and deadlock detection. This project enhances Xinu's synchronization capabilities while ensuring system stability and performance.
