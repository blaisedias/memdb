/*

Copyright (C) 2016  Blaise Dias

This file is part of sqzbsrv.

sqzbsrv is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

sqzbsrv is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with sqzbsrv.  If not, see <http://www.gnu.org/licenses/>.

Classes implementing simple read write locks,
based on 
1) "Fast Userspace Read/Write locks, built on top of mutexes. Paul Mackerras and Rusty Russel."
2) "Futexes are tricky by Ulrich Drepper"
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <sys/time.h>
#include <atomic>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include "bdrwlock.h"


namespace benedias {
const bool verbose = false;

static inline int futex(int *uaddr, int futex_op, int val,
        const struct timespec *timeout, int *uaddr2, int val3)
{
    return syscall(SYS_futex, uaddr, futex_op, val,
                   timeout, uaddr2, val3);
}


static inline int futex_wake(int *uaddr, int wake_count=1)
{
    return futex(uaddr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, wake_count, NULL, NULL, 0);
}


static inline int futex_wait(int *uaddr, int expected)
{
    return futex(uaddr, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, expected, NULL, NULL, 0);
}


void FurwLock1::enter_gate()
{
    // gate values can only be
    // 0 : unlocked
    // 1 : locked
    // 2 : locked contended
    // Transitions on lock 0 -> 1, 1 -> 2, 0 -> 2
    // Transitions on unlock 1 -> 0, 2 -> 0 (wake)
    int expected=0;
    if (!__atomic_compare_exchange_n(&gate, &expected, 1,
               false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
    {
        do
        {
            if (expected == 2 
                    // expected == 1
                    || __atomic_compare_exchange_n(&gate, &expected, 2,
                       false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
                futex_wait(&gate, 2);
            expected = 0;
        } while (!__atomic_compare_exchange_n(&gate, &expected, 2,
                   false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE));
    }
}

void FurwLock1::leave_gate()
{
    int vsampled;
    if ((vsampled = __atomic_fetch_sub(&gate, 1,  __ATOMIC_ACQ_REL)) != 1)
    {
#ifdef  TESTING
        assert(vsampled == 2);
#endif
        // at least one thread is waiting.
        __atomic_store_n(&gate, 0, __ATOMIC_RELEASE);
        futex_wake(&gate, 1);
    }
    else
    {
#ifdef  TESTING
        assert(vsampled == 0);
#endif
    }
}

void FurwLock1::read_lock()
{
    enter_gate();
    // @here if there are no writers
    __atomic_add_fetch(&nreaders, 1, __ATOMIC_RELEASE);
    leave_gate();
}

void FurwLock1::read_unlock()
{
    // Negative if a) this is the last reader
    // b) there is a writer.
    if(0 > __atomic_sub_fetch(&nreaders, 1, __ATOMIC_ACQ_REL))
    {
        futex_wake(&nreaders, 1);
    }
}

void FurwLock1::write_lock()
{
    int val_nreaders;
    enter_gate();
    // gate is unavailable for the duration of the write,
    // including the wait for readers to complete.
    if (0 <= (val_nreaders = __atomic_sub_fetch(&nreaders, 1, __ATOMIC_ACQ_REL)))
    {
        do
        {
            if (0 == futex_wait(&nreaders, val_nreaders))
                break;
            __atomic_load(&nreaders, &val_nreaders, __ATOMIC_CONSUME);
        }while(val_nreaders >= 0);
    }
}

void FurwLock1::write_unlock()
{
    __atomic_store_n(&nreaders, 0, __ATOMIC_RELEASE);
    leave_gate();
}


//============================================================================

void FurwLockSem::read_lock()
{
    sem_wait(&sem);
    // @here if there are no writers
    __atomic_add_fetch(&nreaders, 1, __ATOMIC_ACQ_REL);
    sem_post(&sem);
}

void FurwLockSem::read_unlock()
{
    // Negative if a) this is the last reader
    // b) there is a writer.
    if(0 > __atomic_sub_fetch(&nreaders, 1, __ATOMIC_ACQ_REL))
    {
        futex_wake(&nreaders, 1);
    }
}

void FurwLockSem::write_lock()
{
    int val_nreaders;
    sem_wait(&sem);
    // gate is unavailable for the duration of the write,
    // including the wait for readers to complete.
    if (0 <= (val_nreaders = __atomic_sub_fetch(&nreaders, 1, __ATOMIC_ACQ_REL)))
    {
        do
        {
            if (0 == futex_wait(&nreaders, val_nreaders))
                break;
            __atomic_load(&nreaders, &val_nreaders, __ATOMIC_ACQUIRE);
        }while(val_nreaders >= 0);
    }
}

void FurwLockSem::write_unlock()
{
    __atomic_store_n(&nreaders, 0, __ATOMIC_RELEASE);
    sem_post(&sem);
}

//============================================================================

} // namespace benedias
