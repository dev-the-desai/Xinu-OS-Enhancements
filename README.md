# Xinu-OS-Enhancements
- A collection of system-level enhancements to the Xinu operating system, focusing on core OS functionality including process management, scheduling, and synchronization. 
- Consists of three projects: process management extensions implementing fork and cascading termination, scheduling modifications adding lottery and MLFQ algorithms, and synchronization enhancements incorporating different types of locks with deadlock detection and priority inheritance.

## Xinu OS Process Management Enhancements
Added cascading process termination and Unix-like fork system call to Xinu's process management system. Implemented process stack duplication with distinct parent-child execution paths while preserving memory address space sharing. This project enhances Xinu's process creation and termination capabilities while maintaining system process integrity.

### Key Implementation Features

1. Cascading Process Termination
   - Added distinction between system and user processes
   - Implemented selective cascading termination for user processes
   - Enhanced process kill functionality to handle parent-child relationships
  
2. Fork System Call Implementation
   - Created Unix-like fork() functionality
   - Proper handling of process stack duplication
   - Implemented return value logic:
     - Returns child PID to parent
     - Returns NPROC to child process
     - Returns SYSERR on failure

## Xinu OS Scheduler Modifications
Modified the priority-based scheduler in Xinu to implement Lottery Scheduling and Multi-Level Feedback Queue (MLFQ) scheduling algorithms. This project extends Xinu's process management capabilities while maintaining system process scheduling integrity.

### Key Implementation Features

1. Lottery Scheduler
   - Implemented ticket-based process scheduling
   - Added dynamic ticket allocation functionality
   - Maintained system process priority scheduling
   - Includes fairness analysis capabilities

2. MLFQ Scheduler
   - Implemented multi-queue priority system
   - Added configurable time allotment and priority boost periods
   - Implemented priority boost mechanism
   - Added priority upgrade/downgrade tracking

## Xinu OS Synchronization Extensions
Added synchronization locks (spinlocks) to Xinu and implemented advanced locking mechanisms including bounded wait guards, priority inversion avoidance, and deadlock detection. This project enhances Xinu's synchronization capabilities while ensuring system stability and performance.

### Key Implementation Features

1. Test-and-Set Implementation
   - Custom assembly implementation of atomic test_and_set instruction
   - Based on x86 XCHG instruction
   - Full AT&T syntax compatibility
   - Thoroughly documented assembly code

2. Spinlock Implementation
   - Basic spinlock mechanism using test_and_set
   - Configurable number of system-wide spinlocks
   - System calls for initialization, locking, and unlocking
   - Lightweight synchronization primitive

3. Guard-Protected Locks
   - Advanced locking mechanism with sleep capability
   - Eliminates busy waiting through process parking
   - Custom park/unpark implementation
   - FIFO queue management
   - Priority-aware design to prevent single-lock deadlocks

4. Deadlock Detection
   - Automatic circular dependency detection
   - Real-time deadlock notification system
   - Support for multiple concurrent deadlocks
   - Non-blocking trylock functionality
   - Process dependency tracking

5. Priority Inheritance Protocol
   - Implementation of Basic Priority Inheritance Protocol
   - Dynamic priority adjustment
   - Priority inversion prevention
   - Real-time priority change logging
   - FIFO queue maintenance with priority awareness
