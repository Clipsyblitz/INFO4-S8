#include <nanvix/sem.h>

void sem_init()
{
    struct semtab *sem; 
    
    for (sem = &semtab[0]; sem <= &semtab[PROC_MAX]; sem++)
        sem->state = SEM_DEAD;
}

void create(int n, int id)
{
    // struct semtab s_i = semtab[id];
    struct sem *s = &semtab[id].sem;
    semtab[id].state = SEM_READY;
    s->n = n;
    s->wait_list = NULL;
}

int down(int id, struct process *p)
{
    disable_interrupts();
    struct sem *cs = &(semtab[id].sem);
    if (cs->n > 0)
    {
        enable_interrupts();
        return cs->n--;
    }
    else
    {
        enable_interrupts();
        sleep(&cs->wait_list, p->priority);
    }
    return 0;
}

int up(int id)
{
    disable_interrupts();
    struct sem *cs = &(semtab[id].sem);
    if (cs->n == 0 && cs->wait_list != NULL)
    {
        struct process *p;
        p = cs->wait_list;
        enable_interrupts();
        wakeup_one(&p);
        disable_interrupts();
        cs->wait_list = cs->wait_list->p;
        enable_interrupts();
        return 0;
    }
    else
    {
        cs->n++;
        enable_interrupts();
        return 0;
    }
    enable_interrupts();
    return -1;
}

void destroy(int id)
{
    semtab[id].state = SEM_DEAD;
}
