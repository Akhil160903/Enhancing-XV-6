# **Operating Systems and Networks, Monsoon 2022**
## *Assignment 4 : Enhancing XV-6*

### TEAM :
- Akhil Gupta  : 2021101012
- Harsh Bansal : 2021101027

---

xv6 - Dependent Operating System
An extension of the MIT's xv6 Operating System for RISC-V.
Modifying the xv6 operating system by adding syscalls and modifying the scheduler for process scheduling.

---
### **Running the OS :**
*
      $ make clean
      $ make qemu SCHEDULER=[RR/PBS/FCFS/LBS] CPUS=[N_CPU]
        (DEFAULT SCHEDULER = RR)
        
---

### **Specification 1 : System Calls**
  ### Adding a System Call to xv6
  *
        In syscall.c file :
        [SYS_command_name]  sys_command_name,
        extern int sys_command_name(void);
  *
        In syscall.h file :
        #define SYS_command_name 22
  *
        In sysproc.c file :
        int
        sys_command_name(void) {
        cprintf(“command_name world\n”);
        return 12;
        }
  *     In usys.pl file :
        entry("command_name");
  *
        In user.h file :
        int command_name(void);    
  *
        In Makefile :
        $U_command_name\

*    ### System Call 1 : trace
        * Add the system call trace and an accompanying user program strace . The
          command will be executed as follows:
          *     strace mask command [args]
        * **strace** runs the specified command until it exits.
        * It intercepts and records the system calls which are called by a process during its
          execution.
        *   The line should contain: 
            * The process id
            * The name of the system call
            * The decimal value of the arguments(xv6 passes arguments via registers)
            * The return value of the syscall.

      ### Implementation : ###
      * There is a 'tracemask' defined for the proc struct in kernel/proc.h, which is by default set to 0 in kernel/proc.c. Every time a process is forked, the child inherits its parent's tracemask.
      * Add a user file "strace.c" with strace function
      * Add the syscall strace as defined above
      * Implemented uint64 sys_trace(void); function in kernel/sysproc.c

---
*    ### System Call 1 : sigalarm and sigreturn
        * Add a feature to xv6 that periodically alerts a process as it uses CPU time
        * Implementing a primitive form of user-level interrupt/fault handlers like the SIGCHILD handler in the previous assignment.
        * Add a new sigalarm(interval, handler) system call. If an application calls alarm(n, fn) , then after every n "ticks" of CPU time that the program consumes, the kernel will cause application function fn to be called. When fn returns, the application will resume where it left off.
        * Add another system call sigreturn() , to reset the process state to before the handler was called. This system call needs to be made at the end of the handler so the process can resume where it left off.
      ### Implementation : ###
      * Add sigalarm() and sigreturn() system calls in the usys.pl file.
      ```
      entry("sigalarm");
      entry("sigreturn");
      ```
      * Defining sigalarm and sigreturn to the user.h file.
      ```
      int sigalarm(int ticks, void (*handler)());
      int sigreturn(void);
      ```
      * Added the Function Definitions in the sysproc.c and also added them to the Structure Defined which Stores the Name of the System Call and the Number of Arguements for each of the System call.
      ```
      uint64 sys_sigreturn(void)
      {
      restore();
      myproc()->is_sigalarm = 0;
      return myproc()->trapframe->a0;
      }

      uint64 sys_sigalarm(void)
      {
      int ticks;
      argint(0, &ticks);

      uint64 handler;
      argaddr(1, &handler);

      myproc()->is_sigalarm = 0;
      myproc()->ticks = ticks;
      myproc()->now_ticks = 0;
      myproc()->handler = handler;
      return 0;
      }
      ```
      * Finally Added the user/alarmtest.c for Checking the sigalarm and sigreturn.
      * Run the Following to check Sigalarm and Sigreturn
      ```
      alarmtest
      ```
---
### **Specification 2 : Scheduling**
*     Use the flags for compilation in the following scheduling:-
        ● First Come First Serve = FCFS
        ● Priority Based = PBS
        ● Multilevel Feedback Queue = MLFQ
*     Initialize SCHEDULER = RR (default case) in Makefile

*    ### FCFS (First Come First Serve) :
        * A policy that selects the process with the lowest creation time. The process will run until it no longer needs CPU time.
      ### Implementation : ###
      * Added a variable "creation_t" in proc struct in kernel/proc.h
      * Initialize "creation_t" with ticks in allocproc() in kernel/proc.c
      * Then we schedule the process with the minimum creation time which is currently in the table.
      * The code for FCFS is written in scheduler() in kernel/proc.c within #ifdef FCFS ..... #endif
      * Since this is non-preempted, a condition is added to ignore the yield() in usertrap() in kernel/trap.c when FCFS is defined.
---
*    ### LBS (Lottery Based Scheduler) :
      * A preemptive scheduler that assigns a time slice to the process randomly in proportion to the number of tickets it owns. That is the probability that the process runs in a given time slice is proportional to the number of tickets owned by it.
      * Implement a system call int settickets(int number) , which sets the number of tickets of the calling process. By default, each process should get one ticket.
      * Need to make sure a child process inherits the same number of tickets as its parents.
      ### Implementation : ###
      * Implemented a new system call settickets , which sets the number of tickets of calling process.This done by following above rules :
      * Added a new variables tickets in struct proc in kernel/proc.h
      * Initialize tickets=1 in allocproc() in kernel/proc.c 
      * In fork() in kernel/proc.c add np->tickets=p->tickets; (child should have same tickets as parents)
      * The code for LBS is written in scheduler in kernel/proc.c within #ifdef LBS ..... #endif
      * Then we schedule the process with minimum tickets greater than the random number which is currently in the table.
---
*    ### PBS (Priority Based Scheduler) :
      * A non-preemptive priority-based scheduler that selects the process with the highest priority for execution
      * Use the number of times the process has been scheduled to break the tie. If the tie remains, use the start-time of the process to break the tie
      * There are two types of priorities :
         * Static Priority(SP) can be in range [0,100] Default=60
         * Dynamic Priority(DP) calculated from niceness and SP
         * The niceness is an integer in the range [0, 10] that measures what percentage of the time the process was sleeping
      ### Implementation : ###
      * Implemented a system call named "set_priority" to change the Static Priority
      * Added sys_set_priority() function in kernel/sysproc.c which reschedules the processes if priority of processes increases.
      * Added niceness_flag, st_priority, nsleep ,nrun variables in proc struct in kernel/proc.h 
      * Initialize st_priority=60 ,niceness_flag=5,nsleep =0 ,nrun-0 in allocproc() in kernel/proc.c 
      * yield() function in usertrap() functions in kernel/trap.c is disabled to disable timer interrupts thus disabling preemption.
      * The code for PBS is written in scheduler in kernel/proc.c within #ifdef PBS ..... #endif
      * Then we schedule the process with the highest priority which is currently in the table.
      * To calculate the niceness:
        * Record for how many ticks the process was sleeping and running from the last time it was scheduled by the kernel
        * New processes start with niceness equal to 5. After scheduling the process, compute the niceness as follows:
          *     niceness = Int((Ticks spent in sleeping state / Ticks spent in running + sleeeping state)*10)
      * Use Dynamic Priority to schedule processes which is given as:
         *     DP = max(0, min(100, SP - niceness + 5))

---
### **Comparision between scheduling policies :**

| Scheduler | Avg. Running time | Avg. Waiting time |
| --- | --- | --- |
| RR (default) | 26 | 64 |
| FCFS | 26 | 34 |
| LBS | 27 | 63 |
| PBS | 23 | 49 |

The above running time and scheduling time are calculated by running user/schedulertest.c on 3 CPUs.

---
### **Specification 3 : Copy-on-write fork**
* The idea behind a copy-on-write is that when a parent process creates a child process then both of these processes initially will share the same pages in memory and these shared pages will be marked as copy-on-write
* This means that if any of these processes will try to modify the shared pages then only a copy of these pages will be created and the modifications will be done on the copy of pages by that process and thus not affecting the other process.
* The basic plan in COW fork is for the parent and child to initially share all physical pages, but to map them read-only.
* When the child or parent executes a store instruction, the RISC-V CPU raises a page-fault exception. In response to this exception, the kernel makes a copy of the page that contains the faulted address. It maps one copy read/write in the child’s address space and the other copy read/write in the parent’s address space. 
* After updating the page tables, the kernel resumes the faulting process at the instruction that caused the fault. Because the kernel has updated the relevant PTE to allow writes, the faulting instruction will now execute without a fault.
  
  ### Implementation : ###
  * When fork is called, it calls the function uvmcopy() of kernel/proc.c to generate the exact copy of parent memory block for child. 
  * We change this function so that a copy is not generated and the pages are the pages used by parent are shared. Now to ensure concurrency and prevent faults, we disable writing to this page.
  * Whenever a write operation is performed on such a page, we detect the trap in kernel/trap.c by using r_scause()==15 which indicates store page fault and create a copy of the page where this process can write by enabling write in new page. 
  * The other page remains read only and is deallocated if the number of processes using it become 0. This happens in write_trap function of file kernel/trap.c .
  * A Handle_Cow() Function is implemented which Handles the Allocation of a New Page to the Process.
  ```
      // Implementing a Page Fault While Implementing COW
      int Handle_Cow(pagetable_t Page_Table, uint64 value)
      {
            if (value >= MAXVA)
            {
            return -1;
            }

            pte_t *pte = walk(Page_Table, value, 0);

            // PageTable Not Found
            if (pte == 0)
            {
            return -1;
            }

            if ((*pte & PTE_U) == 0)
            {
            return -1;
            }

            if ((*pte & PTE_V) == 0)
            {
            return -1;
            }

            uint64 page1 = PTE2PA(*pte);

            uint64 page2 = (uint64)kalloc();

            if (page2 == 0)
            {
            printf("KAlloc for COW has Failed\n");
            return -1;
            }

            memmove((void *)page2, (void *)page1, 4096);

            kfree((void *)page1);

            *pte = PA2PTE(page2) | PTE_R | PTE_X | PTE_V | PTE_U | PTE_W;

            return 0;
      }
  ```
  * This Function is called when Page Fault is Observed by the UserTrap Function in a Specific Process. This Function then Handles the Page Fault Accordingly.
  * A new user/cowtest.c was added to the Code which would check whether Copy On Write was Implemented Correctly or Not.
  * Run the Following to check Copy On Write
  ```
  cowtest
  ```
  
---
