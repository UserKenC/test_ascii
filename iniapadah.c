acquire(&ptable.lock);

struct proc *p;
struct proc *highest = 0;

// Find the runnable process with the highest priority
for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
  if(p->state == RUNNABLE){
    if(highest == 0 || p->priority > highest->priority)
      highest = p;
  }
}

// If a highest-priority process is found, run it
if(highest){
  proc = highest;
  switchuvm(highest);
  highest->state = RUNNING;

  swtch(&cpu->scheduler, highest->context);
  switchkvm();

  proc = 0;
}

release(&ptable.lock);
