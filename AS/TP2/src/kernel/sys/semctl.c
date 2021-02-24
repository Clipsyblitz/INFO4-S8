#include <nanvix/sem.h>
#include <nanvix/pm.h>
#include <sys/sem.h>

PUBLIC int sys_semctl(int id, int cmd, int n)
{
    if (semtab[id].state != SEM_DEAD)
    {
        switch (cmd)
        {
        case GETVAL:
            return semtab[id].sem.n;
        case SETVAL:
            semtab[id].sem.n = n;
            return 0;
        case IPC_RMID:
            if (semtab[id].sem.wait_list == NULL)
            {
                destroy(id);
                return 0;
            }
            else
            {
                return -1;
            }
        default:
            return -1;
        }
    }
    else
    {
        return -1;
    }
}