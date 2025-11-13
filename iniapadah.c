int
set_priority(int pid, int new_priority)
{
    struct proc *p;

    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->pid == pid){
            if(new_priority < 0) new_priority = 0;
            if(new_priority > 31) new_priority = 31;
            p->priority = new_priority;
            release(&ptable.lock);

            // preempt if necessary
            yield();  // voluntarily give up CPU if new priority < others
            return 0;
        }
    }
    release(&ptable.lock);
    return -1; // pid not found
}
