#include <nanvix/sem.h>
#include <nanvix/pm.h>

PUBLIC int sys_semop(int id, int op)
{
    if (op >= 0)
    {
        if(up(id) != -1) {
            return 0;
        }
    }
    else
    {
        if (down(id, curr_proc) != -1) {
            return 0;
        }
    }

    return -1;
}