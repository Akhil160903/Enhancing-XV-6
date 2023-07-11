#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
extern int waitx(uint64 addr, uint *rtime, uint *wtime);
extern int set_priority(int process_priority, int process_pid);
// extern int set_priority(int process_priority, int process_pid);
uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

void restore()
{
  struct proc *p = myproc();

  p->trapframe_copy->kernel_satp = p->trapframe->kernel_satp;
  p->trapframe_copy->kernel_sp = p->trapframe->kernel_sp;
  p->trapframe_copy->kernel_trap = p->trapframe->kernel_trap;
  p->trapframe_copy->kernel_hartid = p->trapframe->kernel_hartid;
  *(p->trapframe) = *(p->trapframe_copy);
}

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
uint64
sys_trace(void)
{
  argint(0, &myproc()->tracemask);
  if (myproc()->tracemask < 0)
    return -1;

  return 0;
}
uint64
sys_waitx(void)
{
  uint64 addr, addr1, addr2;
  uint wtime, rtime;
  argaddr(0, &addr);
  if (addr< 0)
    return -1;
  argaddr(1, &addr1);
  if (addr1 < 0) // user virtual memory
    return -1;
  argaddr(2, &addr2);
  if (addr2 < 0)
    return -1;
  int ret = waitx(addr, &wtime, &rtime);
  struct proc *p = myproc();
  if (copyout(p->pagetable, addr1, (char *)&wtime, sizeof(int)) < 0)
    return -1;
  if (copyout(p->pagetable, addr2, (char *)&rtime, sizeof(int)) < 0)
    return -1;
  return ret;
}
uint64
sys_set_priority(void)
{
  int process_priority;
  int process_pid;
  // int ret = -1;
  // struct proc *p;
  // extern struct proc proc[];
  argint(0, &process_priority);
  if (process_priority < 0)
    return -1;
  argint(1, &process_pid);
  if (process_pid < 0)
    return -1;

  // for (p = proc; p < &proc[NPROC]; p++)
  // {
  //   if (p->pid == process_pid)
  //   {
  //     ret = p->st_priority;
  //     p->niceness_flag = 5;
  //     p->lastsleep_t = 0;
  //     p->st_priority = process_priority;
  //   }
  // }
  return set_priority(process_priority,process_pid);
  // return ret;
}
uint64
sys_settickets(void)
{
  int n;
  argint(0, &n);
  if(n < 0)
    return -1;
  myproc()->tickets = n;
  return 1;

}