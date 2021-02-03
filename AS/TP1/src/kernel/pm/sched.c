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

struct process *queue[4] = {NULL};

/**
 * @brief Schedules a process to execution.
 * 
 * @param proc Process to be scheduled.
 */

/*
PUBLIC void sched(struct process *proc)
{
	proc->state = PROC_READY;
	proc->counter = 0;
}
*/
PUBLIC void sched(struct process *proc)
{
	proc->state = PROC_READY;
	proc->counter = 0;
	if (proc != IDLE)
	{
		for (int i = 0; i < 4; i++)
			if (proc->nice <= 10 * (i + 1))
			{
				proc->initial_queue = i;
				break;
			}

		if (proc->current_queue > 3)
			proc->current_queue = proc->initial_queue;

		add_in_queue(proc);
	}
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

PUBLIC void init_queue(struct process *p)
{
	for (int i = 0; i < 4; i++)
		if (p->nice <= 10 * (i + 1))
		{
			p->initial_queue = i;
			break;
		}
	p->current_queue = p->initial_queue;
}

PUBLIC void delete_queue(struct process *p)
{
	struct process *previous = queue[p->current_queue];
	if (previous == p)
		queue[p->current_queue] = NULL;
	else
	{
		for (; previous->queue_next != p; previous = previous->queue_next)
			;
		previous->queue_next = p->queue_next;
	}
}

PUBLIC void add_in_queue(struct process *p)
{
	p->queue_next = queue[p->current_queue];
	queue[p->current_queue] = p;
}

/**
 * @brief Yields the processor.
 */
PUBLIC void yield(void)
{
	struct process *p;	  /* Working process.     */
	struct process *next; /* Next process to run. */

	/* Re-schedule process for execution. */
	if (curr_proc->state == PROC_RUNNING)
	{
		if (curr_proc != IDLE)
		{
			curr_proc->current_queue++;
		}
		sched(curr_proc);
	}

	/* Remember this process. */
	last_proc = curr_proc;

	/* Check alarm */
	for (p = FIRST_PROC; p != LAST_PROC; p++)
	{
		if (!IS_VALID(p))
			continue;

		if ((p->alarm) && (p->alarm < ticks))
			p->alarm = 0, sndsig(p, SIGALRM);
	}

	/* Choose a process to run next. */
	next = IDLE;
	next->counter = 0;

#define P 1
#define C -1
	int i = 0;
	int prio_p;
	int prio_next;

	for (; i < 4; i++)
	{

		p = queue[i];
		while (p != NULL)
		{
			prio_p = P * p->priority + C * p->counter;
			prio_next = P * next->priority + C * next->counter;

			if (prio_p < prio_next)
			{
				next->counter++;
				next = p;
			}
			else if (prio_p == prio_next)
			{
				if (p->nice <= next->nice)
				{
					next->counter++;
					next = p;
				}
				else
					p->counter++;
			}
			else
				p->counter++;

			p = p->queue_next;
		}

		if (next != IDLE)
			break;
	}

	if (next != IDLE)
		delete_queue(next);

	/* Switch to next process. */
	next->priority = PRIO_USER;
	next->state = PROC_RUNNING;
	next->counter = PROC_QUANTUM;

	if (curr_proc != next)
		switch_to(next);
}