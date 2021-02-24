#include <nanvix/sem.h>
#include <nanvix/pm.h>

PUBLIC int sys_semget(unsigned key)
{
    int id_libre = -1;
    for (int i = 0; i < PROC_MAX; i++)
    {
        if (semtab[i].key == key && semtab[i].state == SEM_READY)
            return i;
        if (semtab[i].state == SEM_DEAD)
            id_libre = i;
    }
    if (id_libre == -1)
    {
        return -1;
    }
    create(0, id_libre);
    semtab[id_libre].key = key;
    return id_libre;
}