/*

Copyright (C) 2017,2018  Blaise Dias

sharedobj is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

It is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with sharedobj.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BENEDIAS_SHARED_OBJ_H
#define BENEDIAS_SHARED_OBJ_H
#include <atomic>
#include <utility>

//#define SHARED_OBJ_DEBUG_TRACE
#ifdef  SHARED_OBJ_DEBUG_TRACE
#include <iostream>
#ifndef sharedobj_traceout
#define sharedobj_traceout std::cout
#endif
#endif
namespace benedias {

//sharedobj_ptr class template implements a subset of std::shared_ptr functionality,
//for classes subclassing the sharedobj (see below).
//The only advantages of this class over std::shared_ptr is the size of
//sharedobj_ptr, which is the same as that of a pointer, and possibly its simplicity.
//The reference counter in the sharedobj class template is atomic, and
//sharedobj methods are thread safe.
//However this is not true for sharedobj_ptr, this is intentional.
//There is no need to share sharedobj_ptr instances between threads,
//creating new instances is cheap in terms of size (the primary motive for the
//existence of sharedobj_ptr) and not too expensive in terms of execution.
//sharedobj_ptr should be passed between threads by copying,
//i.e. make a copy and pass that to another thread.
//Q: What about models where a central shared repository of sharedobj_ptrs
//is used?


//  @brief  A smart pointer to classes derived from sharedobj or implementing
//  the sharedobj interface, with reference-counted copy semantics.
//
//  The object pointed to is deleted when there are no sharedobj_ptr pointing to it.
template <typename T> class sharedobj_ptr
{
    private:
        T* real_ptr = nullptr;
        inline T* acquire() const
        {
            if (real_ptr)
            {
                real_ptr->sharedobj_acquire();
                return real_ptr;
            }
            return nullptr;
        }

        inline void release()
        {
            if (real_ptr)
            {
                if (real_ptr->sharedobj_release())
                {
#ifdef  SHARED_OBJ_DEBUG_TRACE
                    sharedobj_traceout << "deleting  " << real_ptr << " @release" << std::endl;
#endif
                    delete real_ptr;
                }
                real_ptr = nullptr;
            }
        }

        T* get_n_zap()
        {
            T* tmp = real_ptr;
            real_ptr = nullptr;
            return tmp;
        }
    public:
        //(constructor)

        // @brief   Consructor an empty sharedobj_ptr
        sharedobj_ptr()
        {
        }

        // @brief   Consructor an sharedobj_ptr instance that shares ownership with @a sp_other.
        sharedobj_ptr(const sharedobj_ptr<T>& sp_other)
        {
            real_ptr = sp_other.acquire();
        }

        // @brief   Consructor an sharedobj_ptr instance that takes ownership from @a sp_other.
        sharedobj_ptr(sharedobj_ptr<T>&& sp_other)
        {
//            using std::move this breaks! sp_other.real_ptr is not cleared
//            real_ptr = std::move(sp_other.real_ptr);
//            adding this line below fixes it.
//            sp_other.real_ptr = nullptr;

//            real_ptr = sp_other.acquire();
//            sp_other.real_ptr = sp_other.release();

            real_ptr = sp_other.get_n_zap();
        }

        // @brief   Consructor an sharedobj_ptr instance that points to @a ptr.
        sharedobj_ptr(T* ptr)
        {
            if (ptr)
            {
                ptr->sharedobj_acquire();
                real_ptr = ptr;
            }
        }

        //(destructor)

        //@brief    If this instance owns an object and it is the last sharedobj_ptr owning it,
        //the object is destroyed.
        // After the destruction, the smart pointers that shared ownership with this instance,
        // if any, will report a use_count() that is one less than its previous value.
        ~sharedobj_ptr()
        {
            release();
        }

        //@brief     return true if the sharedobj_ptr instance is pointing to an object.
        explicit operator bool() const noexcept
        {
            return real_ptr != nullptr;
        }

        //Dereferencing operators

        //@brief     Dereferences the stored pointer. The behavior is undefined if the stored pointer is null.
        T* operator->() const noexcept
        {
            return real_ptr;
        }

        //@brief     Dereferences the stored pointer. The behavior is undefined if the stored pointer is null.
        T operator*() const noexcept
        {
            return *real_ptr;
        }

        //Assignment operators

        //@brief    Replaces the managed object with the one managed by sp_other.
        //If this instance owns an object, destructor semantics apply.
        // After the assignment, the smart pointers that shared ownership with this instance,
        // if any, will report a use_count() that is one more than its previous value.
        void operator=(const sharedobj_ptr<T>& sp_other)
        {
            release();
            real_ptr = sp_other.acquire();
        }

        //@brief    Move-assigns the managed object with the one managed by @a sp_other.
        // If this instance owns an object, destructor semantics apply.
        // After the assignment, the smart pointers that shared ownership with this instance,
        // if any, will report a use_count() that is the same as its previous value.
        void operator=(sharedobj_ptr<T>&& sp_other)
        {
            release();
//            using std::move this breaks! sp_other.real_ptr is not cleared
//            real_ptr = std::move(sp_other.real_ptr);

//            real_ptr = sp_other.acquire();
//            sp_other.real_ptr = sp_other.release();

            real_ptr = sp_other.get_n_zap();
        }

        // Equality operators

        //@brief    Returns true if the managed object of @a sp_other is the
        // same as the managed object of this instance.
        bool operator==(const sharedobj_ptr<T>& sp_other) const
        {
            return real_ptr == sp_other.real_ptr;
        }

        //@brief    Returns true if the managed object of @a sp_other is not
        // the same as the managed object of this instance.
        bool operator!=(const sharedobj_ptr<T>& sp_other) const
        {
            return real_ptr != sp_other.real_ptr;
        }

        //@brief    Returns true if the pointer value of the managed object of
        // @a sp_other is less then pointer value to the managed object of this
        // instance.
        bool operator<(const sharedobj_ptr<T>& sp_other) const
        {
            return real_ptr < sp_other.real_ptr;
        }

        // Utility

        //@brief    Returns the pointer to the managed object.
        T* get() const
        {
            return real_ptr;
        }

        //@brief    Returns the pointer value to this instance.
        const void *self() const
        {
            return this;
        }

        //@brief    Returns the number of sharedobj_ptr instances referring to
        // the same object.
        unsigned int use_count() const
        {
            return real_ptr->sharedobj_use_count();
        }
};

//sharedobj class template implements an interface required by sharedobj_ptr implementation,
//for reference counting the "liveness" of an object.
template <typename T> class sharedobj {
    private:
    protected:
        friend sharedobj_ptr<T>;
        std::atomic_uint refcount;

        //@brief    atomically increment the use count of the shared object.
        void sharedobj_acquire()
        {
            ++refcount;
#ifdef  SHARED_OBJ_DEBUG_TRACE
            sharedobj_traceout << "sharedobj_acquire " << this << " " << refcount << std::endl;
#endif
        }

        //@brief    atomically decrement the use count of the shared object.
        //@return   true if the instance should be destroyed.
        bool sharedobj_release()
        {
#ifdef  SHARED_OBJ_DEBUG_TRACE
            sharedobj_traceout << "sharedobj_release " << this << " " << refcount << std::endl;
#endif
            return 0 == --refcount;
        }

        //@brief    Returns the number of sharedobj_ptr instances referring to
        // the same object.
        unsigned int sharedobj_use_count() const
        {
            return refcount.load();
        }

        //@brief    Constructor of the sharedobj, use count is set to 0.
        sharedobj():refcount(0)
        {
#ifdef  SHARED_OBJ_DEBUG_TRACE
            sharedobj_traceout << "CTOR sharedobj " << this << " " << refcount << std::endl;
#endif
        }

        //@brief    Destructor of the sharedobj, use count must be 0.
        virtual ~sharedobj()
        {
#ifdef  SHARED_OBJ_DEBUG_TRACE
            sharedobj_traceout << "DTOR sharedobj " << this << " " << refcount << std::endl;
#endif
        }
};

//@brief    Allocates and constructs and object of type @a T and returns a
//object of type @a sharedobj_ptr<T>.
template <typename T, typename... Args> sharedobj_ptr<T> make_sharedobj(Args&&... args)
{
    T* ptr = ::new T(std::forward<Args>(args)...);
    return sharedobj_ptr<T>(ptr);
}

} //namespace benedias

namespace std
{
    template <typename T>
    struct hash<benedias::sharedobj_ptr<T>>
    {
        size_t operator()(benedias::sharedobj_ptr<T>& rcp) const
        {
            return std::hash<T>()(*rcp.get());
        }
    };
} //namespace std
#endif // #define BENEDIAS_SHARED_OBJ_H

