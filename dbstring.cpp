/*

Copyright (C) 2017,2018  Blaise Dias

This file is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This file is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this file.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <mutex>
#include <stack>
#include <unordered_map>
#include <bitset>
#include <string.h>
#include <memory>
#include <assert.h>
#include <thread>

//#define DBSTRING_DEBUG_TRACE
#ifdef  DBSTRING_DEBUG_TRACE
#include <iostream>
static std::ostream& trace_out(std::cout);
#endif

#include "bdrwlock.h"
#include "dbstring.h"
#include "lookup3.h"

namespace benedias {
    namespace memdb {

static std::mutex domain_factory_mutex; 
static std::bitset<BENEDIAS_MEMDB_DBSTRING_MAX_DOMAIN_COUNT> domain_master;
static bool reap_immediately = true;

#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
class dbstring_domain
{
    unsigned bitnum;
    public:
        std::bitset<BENEDIAS_MEMDB_DBSTRING_MAX_DOMAIN_COUNT> mask;
        dbstring_domain()
        {
            std::lock_guard<std::mutex> lockg(domain_factory_mutex);
            for (bitnum = 0; bitnum < BENEDIAS_MEMDB_DBSTRING_MAX_DOMAIN_COUNT; ++bitnum)
            {
                if (!domain_master.test(bitnum))
                    break;
            }
            if (bitnum >= BENEDIAS_MEMDB_DBSTRING_MAX_DOMAIN_COUNT)
                throw std::overflow_error("benedias::memdb::dbstring domains");
            domain_master[bitnum] = true;
            mask[bitnum] = true;
        }
        ~dbstring_domain()
        {
            std::lock_guard<std::mutex> lockg(domain_factory_mutex);
            domain_master[bitnum] = false;
        }
};
#endif

struct cstr_compare : public std::binary_function<const char*, const char*, bool>
{
    bool operator()(const char* lhs, const char* rhs)
    {
        return strcmp(lhs,rhs) < 0;
    }
};

struct cstr_equal_to : public std::binary_function<const char*, const char*, bool>
{
    bool operator()(const char* lhs, const char* rhs) const
    {
        if (lhs == rhs)
            return true;
        if ((lhs == 0) || (rhs == 0))
            return false; 
        return strcmp(lhs,rhs) == 0;
    }
};

struct cstr_hash
{
    std::size_t operator()(const char* cs) const
    {
//        return std::hash<std::string>()(std::string(cs));
#ifndef LOOKUP3_H_INCLUDED
        std::size_t h = 0;
        const unsigned char* ucs = (const unsigned char*)cs;

        if (ucs)
        {
            while(*ucs)
            {
                h += *ucs;
                h += (h << 10);
                h ^= (h >> 6);
                ++ucs;
            }
    
            h += (h << 3);
            h ^= (h >> 11);
            h += (h << 15);
        }
        return h;
#else
        std::size_t hashv = (std::size_t)47 + ((std::size_t)13 << 32);
        if (sizeof(hashv) == 8)
        {
            uint32_t* p = (uint32_t *)&hashv;
            lkp3_hash2(cs, strlen(cs), p, p+1);
        }
        else
        {
            hashv = lkp3_hash(cs, strlen(cs), 47);
        }
        return hashv;
#endif
    }
};

//For 100% safety, the lifetime of C string instances used for keys must either 
// 1) span the lifetime of the associated entry string_directory
//or
// 2) exceed the lifetime of string_directory
// Therefore adding strings must be done with care.
// Perversely for this use case, setting the "key" to char_seq in the db_string instance
// is the solution. There isn't a circular dependency because the dbstring instance is
// guaranteed to exist as long as ther is a single sharedobj_ptr instance, and one exists
// within the unordered map, and "can" only be deleted after the entry in string_directory
// is removed.
typedef std::unordered_map<const char*, sharedobj_ptr<dbstring>, cstr_hash, cstr_equal_to> HASHTABLE;
static HASHTABLE string_directory;
#if 0
typedef std::unordered_map<const char*, dbstring, cstr_hash, cstr_equal_to> HASHTABLE2;
static HASHTABLE2 shadow;
#endif
benedias::FurwLock1 string_directory_lock;

#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
static std::shared_ptr<dbstring_domain> no_domain;
static thread_local std::shared_ptr<dbstring_domain> th_domain = no_domain;
#endif

class dbstring_iterator_state
{
    private:
        HASHTABLE::const_iterator itr;
#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
        std::shared_ptr<dbstring_domain> dom;
#endif
        friend dbstring_iterator;
    public:
        dbstring_iterator_state(HASHTABLE::const_iterator& itr_in)
        {
            itr = itr_in;
#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
            seek_valid();
            dom = no_domain;
#endif
        }
#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
        dbstring_iterator_state(HASHTABLE::const_iterator& itr_in, std::shared_ptr<dbstring_domain>dom_in)
        {
            itr = itr_in;
            dom = dom_in;
            seek_valid();
        }

        void seek_valid()
        {
            if (dom && dom->mask.any())
            {
                while(itr != string_directory.cend() && (itr->second->domains &= dom->mask).none())
                {
                    ++itr;
                }
            }
        }
#endif
};

bool dbstring_iterator::operator==(const dbstring_iterator& other)
{
    return state->itr == other.state->itr;
}

bool dbstring_iterator::operator!=(const dbstring_iterator& other)
{
    return state->itr != other.state->itr;
}

sharedobj_ptr<dbstring> dbstring_iterator::operator*()
{
    return state->itr->second;
}

dbstring_iterator& dbstring_iterator::operator++()
{
    ++(state->itr);
#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
    state->seek_valid();
#endif
    return *this;
}


// class static functions
dbstring_iterator dbstring::begin()
{
    HASHTABLE::const_iterator itr = string_directory.cbegin();
    return dbstring_iterator(std::make_shared<dbstring_iterator_state>(itr));
}

#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
dbstring_iterator dbstring::begin(std::shared_ptr<dbstring_domain> dom_in)
{
    HASHTABLE::const_iterator itr = string_directory.cbegin();
    return dbstring_iterator(std::make_shared<dbstring_iterator_state>(itr, dom_in));
}
#endif


dbstring_iterator dbstring::end()
{
    HASHTABLE::const_iterator itr = string_directory.end();
    return dbstring_iterator(std::make_shared<dbstring_iterator_state>(itr));
}

sharedobj_ptr<dbstring> dbstring::make_sharedobj(const char* chars_in)
{
#ifdef  DBSTRING_DEBUG_TRACE
    bool created = false;
#endif
    sharedobj_ptr<dbstring> rv;
    // readlock 
    {
        benedias::read_lock_guard   rdlockg(string_directory_lock);
        HASHTABLE::iterator find = string_directory.find(chars_in);
        if (find != string_directory.end())
        {
            rv = find->second;
        }
    }
    // readlock
    if (!rv)
    {
        sharedobj_ptr<dbstring> dbs(new dbstring(chars_in));
        // writelock
        {
            benedias::write_lock_guard   wrlockg(string_directory_lock);
            HASHTABLE::iterator find = string_directory.find(chars_in);
            if (find == string_directory.end())
            {
                string_directory[dbs->char_seq] = dbs;
                rv = string_directory[dbs->char_seq];
#ifdef  DBSTRING_DEBUG_TRACE
                created = true;
#endif
            }
            else
            {
                rv = find->second;
            }
        }
        // writelock
#ifdef  DBSTRING_DEBUG_TRACE
    if (created)
    {
        trace_out << "benedias::memdb::dbstring CREATED " << dbs.get() << " " << dbs->char_seq << std::endl;
    }
#endif
    }
    return rv;
}

#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
sharedobj_ptr<dbstring> dbstring::make_sharedobj(const char* chars_in, std::shared_ptr<dbstring_domain>& dom)
{
    sharedobj_ptr<dbstring> rv = dbstring::make_sharedobj(chars_in);
    if (dom)
    {
        rv->domains |= dom->mask;
    }
    return rv;
}
#endif

void dbstring::delay_reap(bool v)
{
    reap_immediately = !v;
}

bool dbstring::is_reap_delayed()
{
    return !reap_immediately;
}

void dbstring::reap()
{
    std::stack<sharedobj_ptr<dbstring>> candidates;
    {
        benedias::read_lock_guard rdlockg(string_directory_lock);
        for(HASHTABLE::const_iterator it = string_directory.cbegin(); it != string_directory.cend(); ++it)
        {
            if (it->second.use_count() == 1)
            {
                candidates.push(it->second);
            }
        }
    }
    {
        benedias::write_lock_guard rdlockg(string_directory_lock);
        while(!candidates.empty())
        {
            sharedobj_ptr<dbstring> dbs = candidates.top();
            candidates.pop();
            assert(dbs.use_count() >= 2);
            // 2 => 
            if (dbs.use_count() == 2)
            {
                string_directory.erase(dbs->char_seq);
#ifdef  DBSTRING_DEBUG_TRACE
        trace_out << "benedias::memdb::dbstring.reaped " << dbs.get() << " " << dbs->char_seq << std::endl;
#endif
            }
        }
    }
}

#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
std::shared_ptr<dbstring_domain> dbstring::get_domain()
{
    return std::make_shared<dbstring_domain>();
}
#endif

// member functions
// CTOR
dbstring::dbstring(const char* chars_in):refcount(0)
{
    char* chars = new char[strlen(chars_in) + 1];
    strcpy(chars, chars_in);
    char_seq = chars;
#ifdef  DBSTRING_DEBUG_TRACE
    trace_out << "benedias::memdb::dbstring CTOR " << this << " " << char_seq << std::endl;
#endif
}

// DTOR
dbstring::~dbstring()
{
#ifdef  DBSTRING_DEBUG_TRACE
    trace_out << "benedias::memdb::dbstring DTOR " << this << " " << char_seq << std::endl;
#endif
}
// Comparators
bool dbstring::operator==(const dbstring& other) const
{
    return 0 == strcmp(char_seq, other.char_seq);
}

bool dbstring::operator==(const char* c_str) const
{
    return 0 == strcmp(char_seq, c_str);
}

bool dbstring::operator==(const std::string& cpp_str) const
{
    return 0 == strcmp(char_seq, cpp_str.c_str());
}

bool dbstring::operator<(const dbstring& other) const
{
    if (char_seq == other.char_seq)
        return false;
    return 0 > strcmp(char_seq, other);
}

bool dbstring::operator>(const dbstring& other) const
{
    return 0 < strcmp(char_seq, other);
}

bool dbstring::operator<(const char* c_str) const
{
    return 0 > strcmp(char_seq, c_str);
}

//
size_t dbstring::size() const
{
    return strlen(char_seq);
}

std::size_t dbstring::hash_value() const
{
    return cstr_hash()(char_seq);
}

bool dbstring::sharedobj_release()
{
#ifdef  SHARED_OBJ_DEBUG_TRACE
    sharedobj_traceout << "sharedobj_release " << this << " " << (refcount) << std::endl;
#endif
    // If we want immediate release of memory resources, then we have to be careful.
    unsigned int rc = --refcount;
    
    if ((1 == rc) && (reap_immediately))
    {
//trace_out << "sharedobj_release " << this << " " << (refcount) << " " << reap_immediately <<  std::endl;
        // use count of 2 => string_directory shared_ptr and the caller,
        // which is relinquishing sharing pointing to this object,
        // so this is a candidate for deletion.
        // Remove the string from the directory in a thread safe way.
        benedias::write_lock_guard rdlockg(string_directory_lock);
        // Have lock, check again remove only if this is still  a candidate
        // for deletion.
        if (1 == refcount.load())
        {
            string_directory.erase(char_seq);
            // @here the instance will have been deleted, return false,
            // to avoid double deletions.
            return false;
        }
    }
    return 0 == rc;
}

void dbstring::sharedobj_acquire()
{
    ++refcount;
#ifdef  SHARED_OBJ_DEBUG_TRACE
    sharedobj_traceout << "sharedobj_acquire " << this << " " << refcount << std::endl;
#endif    
}

unsigned int dbstring::sharedobj_use_count() const
{
    return refcount.load();
}


//template <class... Args> sharedobj_ptr<benedias::memdb::dbstring> make_sharedobj(Args&&... args)
//{
//    benedias::memdb::dbstring* ptr = ::new benedias::memdb::dbstring(std::forward<Args>(args)...);
//    trace_out << "sharedobj_ptr<benedias::memdb::dbstring> make_sharedobj .............. " << std::endl;
//    return sharedobj_ptr<benedias::memdb::dbstring>(ptr);
//}

sharedobj_ptr<benedias::memdb::dbstring> make_sharedobj(const char* chars)
{
#ifdef  DBSTRING_DEBUG_TRACE
    trace_out << "sharedobj_ptr<benedias::memdb::dbstring> make_sharedobj .............. " << std::endl;
#endif
#if 0
    benedias::memdb::dbstring* ptr = ::new benedias::memdb::dbstring(std::forward<const char*>(chars));
    return sharedobj_ptr<benedias::memdb::dbstring>(ptr);
#else
    return benedias::memdb::dbstring::make_sharedobj(chars);
#endif
}
    } // namespace memdb

} // namespace benedias
