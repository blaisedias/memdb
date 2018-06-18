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

The rules are simple and harsh, acquiring the same lock read or write more than
once in the same thread *WILL* lock up.
    1) read lock followed by write lock, will deadlock since write locking waits for
    current reads to end.
    2) read lock followed by read lock, will deadlock if a write lock starts between the
    the read lock operations.
    3) write lock followed by read lock, will deadlock since read locking after a write
    lock has started will block till the write has completed.
    4) write lock followed by write lock, will deadlock since write locking waits for
    previous writes to complete.

    This is good enough for our purposes!
    The exception to these rules may be PthreadRWLock, which uses pthread_rwlock.
*/
#include <semaphore.h>
#include <pthread.h>

namespace benedias {
class RWLock
{
    public:
        virtual ~RWLock(){}

        virtual void read_lock()=0;
        virtual void read_unlock()=0;
        virtual void write_lock()=0;
        virtual void write_unlock()=0;
};

// Simple FIFO read write lock, using a pair of futexes and atomic ops.
// "FIFO" means first in first out, writers are not starved.
class FurwLock1:public RWLock
{
    protected:
        int gate = 0;
        int nreaders = 0;

        void enter_gate();
        void leave_gate();
    public:
        FurwLock1():gate(0),nreaders(0){}
        ~FurwLock1(){}

        void read_lock();
        void read_unlock();
        void write_lock();
        void write_unlock();

};


// Simple FIFO read write lock, using a futexes, a semaphore and atomic ops.
// "FIFO" means first in first out, writers are not starved.
class FurwLockSem:public RWLock
{
    protected:
        sem_t  sem;
        int nreaders;
    public:
        FurwLockSem()
        {
            sem_init(&sem, 0, 1);
            nreaders = 0;
        }

        ~FurwLockSem()
        {
            sem_destroy(&sem);
        }

        void read_lock();
        void read_unlock();
        void write_lock();
        void write_unlock();
};

// Simplistic wrapping of pthread_rwlock.
// Default attributes, which means writers can be starved.
class PthreadRWLock:public RWLock
{
    protected:
        pthread_rwlock_t rwlock;
    public:
        PthreadRWLock()
        {
            pthread_rwlockattr_t    attr;
            pthread_rwlockattr_init(&attr);
            pthread_rwlock_init(&rwlock, NULL);
        }

        ~PthreadRWLock()
        {
            pthread_rwlock_destroy(&rwlock);
        }

        void read_lock() { pthread_rwlock_rdlock(&this->rwlock);}
        void read_unlock() { pthread_rwlock_unlock(&this->rwlock);}
        void write_lock() { pthread_rwlock_wrlock(&this->rwlock);}
        void write_unlock() { pthread_rwlock_unlock(&this->rwlock);}
};


// Read lock guard impl
class read_lock_guard
{
    RWLock&  fu_rw_lock;
    bool locked;
    public:
        read_lock_guard(RWLock& fl):fu_rw_lock(fl), locked(true)
        {
            fu_rw_lock.read_lock();
        }

        ~read_lock_guard()
        {
            unlock();
        }

        void unlock()
        {
            if (locked)
            {
                locked = false;
                fu_rw_lock.read_unlock();
            }
        }
};

// Write lock guard impl
class write_lock_guard
{
    RWLock&  fu_rw_lock;
    bool locked;
    public:
        write_lock_guard(RWLock& fl):fu_rw_lock(fl), locked(true)
        {
            fu_rw_lock.write_lock();
        }

        ~write_lock_guard()
        {
            unlock();
        }

        void unlock()
        {
            if (locked)
            {
                locked = false;
                fu_rw_lock.write_unlock();
            }
        }
};

} // namespace benedias
