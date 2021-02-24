#ifndef __SEM_HH__
#define __SEM_HH__

#include <nanvix/pm.h>

#ifndef _ASM_FILE_

#define SEM_READY 1
#define SEM_DEAD 0

struct sem
{
    int n;
    struct process *wait_list;
};

struct semtab {
    unsigned state;
    unsigned key;
    struct sem sem;
};

struct semtab semtab[PROC_MAX];

EXTERN void sem_init();

EXTERN void create(int n, int id);

EXTERN int down(int id, struct process *p);

EXTERN int up(int id);

EXTERN void destroy(int id);

#endif

#endif