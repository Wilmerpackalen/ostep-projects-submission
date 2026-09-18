#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "syscall.h"

// defs.h: void countinit(void); void count_syscall(int);
// Makefile UPROGS: _testreadcount

// laskuri ja mikä syscall on seurannassa
struct spinlock countlock;
int scount;
int seuranta;

void
countinit(void)
{
  initlock(&countlock, "scount");
  scount = 0;
  seuranta = SYS_read; // oletuksena readit
}

void
count_syscall(int num)
{
  acquire(&countlock);
  if(num == seuranta)
    scount++;
  release(&countlock);
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// 0 = palauta laskuri
// -1 = nollaa
// >0 = vaihda seurattava syscall (5=read, 16=write, ...)
int
sys_getreadcount(void)
{
  int cmd;
  int ret;

  if(argint(0, &cmd) < 0)
    return -1;

  acquire(&countlock);
  if(cmd < 0){
    scount = 0;
    ret = 0;
  } else if(cmd > 0){
    seuranta = cmd;
    scount = 0;
    ret = 0;
  } else {
    ret = scount;
  }
  release(&countlock);
  return ret;
}
