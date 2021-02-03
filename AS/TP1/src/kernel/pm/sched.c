/*
 * Copyright(C) 2011-2016 Pedro H. Penna   <pedrohenriquepenna@gmail.com>
 *              2015-2016 Davidson Francis <davidsondfgl@hotmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/clock.h>
#include <nanvix/const.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <signal.h>
#include <nanvix/klib.h>
#define SELECT_CONST 100
#define P -1
#define N -1

struct process *process_max = NULL;

/**
 * @brief Schedules a process to execution.
 * 
 * @param proc Process to be scheduled.
 */
PUBLIC void sched(struct process *proc)
{
	proc->state = PROC_READY;
	proc->counter = 0;
	int nb_tickets = SELECT_CONST + P * proc->priority + N * proc->nice;

	if (process_max == NULL)
	{
		process_max = proc;
	}
	else
	{
		if (process_max->lottery_tickets < nb_tickets)
		{
			process_max = proc;
		}
	}

	proc->lottery_tickets = nb_tickets;
}

/**
 * @brief Stops the current running process.
 */
PUBLIC void stop(void)
{
	curr_proc->state = PROC_STOPPED;
	sndsig(curr_proc->father, SIGCHLD);
	yield();
}

/**
 * @brief Resumes a process.
 * 
 * @param proc Process to be resumed.
 * 
 * @note The process must stopped to be resumed.
 */
PUBLIC void resume(struct process *proc)
{
	/* Resume only if process has stopped. */
	if (proc->state == PROC_STOPPED)
		sched(proc);
}

/**
 * @brief Yields the processor.
 */
PUBLIC void yield(void)
{
	struct process *p;	  /* Working process.     */
	struct process *next; /* Next process to run. */

	if (!IS_VALID(process_max))
	{
		process_max = FIRST_PROC;
		for (p = FIRST_PROC; p < LAST_PROC; p++)
		{
			if (!IS_VALID(p))
			{
				continue;
			}

			if (p->lottery_tickets > process_max->lottery_tickets)
			{
				process_max = p;
			}
		}
	}


	/* Re-schedule process for execution. */
	if (curr_proc->state == PROC_RUNNING)
	{
		sched(curr_proc);
	}

	/* Check alarm */
	for (p = FIRST_PROC; p != LAST_PROC; p++)
	{
		if (!IS_VALID(p))
		{
			continue;
		}

		if ((p->alarm) && (p->alarm < ticks))
			p->alarm = 0, sndsig(p, SIGALRM);
	}

	next = IDLE;
	next->counter = 0;
	int ticket_choice = krand()%process_max->lottery_tickets; //RANDOM
	for (p = FIRST_PROC; p != LAST_PROC; p++)
	{
		if (p->state != PROC_READY)
		{
			continue;
		}

		if (p->lottery_tickets >= ticket_choice)
		{
			if (next->lottery_tickets < p->lottery_tickets)
			{
				next->counter++;
				next = p;
			}
			else
			{
				if (next->lottery_tickets == p->lottery_tickets)
				{
					if (next->counter <= p->counter)
					{
						next->counter++;
						next = p;
					}
					else
					{
						p->counter++;
					}
				}
				else
				{
					p->counter++;
				}
			}
		}
		else
		{
			p->counter++;
		}
	}

	/* Switch to next process. */
	next->priority = PRIO_USER;
	next->state = PROC_RUNNING;
	next->counter = PROC_QUANTUM;

	if (curr_proc != next)
		switch_to(next);
}