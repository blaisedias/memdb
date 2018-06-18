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
#ifndef BENEDIAS_DB_STRING_H_INCLUDED
#define BENEDIAS_DB_STRING_H_INCLUDED

#include <memory>
#include <ostream>
#include <bitset>
#include <atomic>

#include "sharedobj.h"

#ifndef BENEDIAS_MEMDB_DBSTRING_MAX_DOMAIN_COUNT
#define BENEDIAS_MEMDB_DBSTRING_MAX_DOMAIN_COUNT    32
#endif

//#define BENEDIAS_USE_DBSTRING_DOMAINS

namespace benedias {
    namespace memdb {
        // Domains are used "mark" strings as belonging to a set.
        // A string can belong to multiple domains.
        // Used for filtering during serialization,
        // domain values are not persistent across multiple runs,
        // and do not need to be serialised.
#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
        class dbstring_domain;
#endif
        class dbstring_iterator;

        //@brief    immutable string object for in memory database use.
        //class dbstring:public benedias::sharedobj<dbstring>
        class dbstring
        {
            private:
                std::atomic_uint refcount;

                // To reduce the size impact, we explicitly implement sharedobj
                // interface, instead of inheriting it.
                friend benedias::sharedobj_ptr<benedias::memdb::dbstring>;
                void sharedobj_acquire();
                bool sharedobj_release();
                unsigned int sharedobj_use_count() const;

                const char* char_seq;
#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
                std::bitset<BENEDIAS_MEMDB_DBSTRING_MAX_DOMAIN_COUNT> domains;
#endif
                // Non copyable
                dbstring& operator=(const dbstring&) = delete;
                dbstring(dbstring const&) = delete;

                // Non movable
                dbstring& operator=(dbstring&&) = delete;
                dbstring(dbstring&&) = delete;
                friend class dbstring_iterator_state;
//                template <class... Args> friend sharedobj_ptr<benedias::memdb::dbstring> make_sharedobj(Args&&... args);
            protected:
                dbstring(const char*);
                friend sharedobj_ptr<benedias::memdb::dbstring> make_sharedobj(const char* args);
            public:
                ~dbstring();

                // CTOR factory methods.
                static sharedobj_ptr<dbstring> make_sharedobj(const char* chars);
                static sharedobj_ptr<dbstring> make_sharedobj(const std::string& stdstr)
                {
                    return dbstring::make_sharedobj(stdstr.c_str());
                }
#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
                static sharedobj_ptr<dbstring> make_sharedobj(const char* chars, std::shared_ptr<dbstring_domain>& dom);
                static sharedobj_ptr<dbstring> make_sharedobj(const std::string& stdstr, std::shared_ptr<dbstring_domain>& dom)
                {
                    return dbstring::make_sharedobj(stdstr.c_str(), dom);
                }

                static std::shared_ptr<dbstring_domain> get_domain();
                static dbstring_iterator begin(std::shared_ptr<dbstring_domain>);
#endif
                static void delay_reap(bool);
                static bool is_reap_delayed();

                static dbstring_iterator begin();
                static dbstring_iterator end();

                static void reap();

                // Comparators
                bool operator==(const dbstring& dbstr) const;
                bool operator==(const char* c_str) const;
                bool operator==(const std::string& cpp_str) const;

                bool operator<(const char* c_str) const;
                bool operator<(const dbstring& dbstr) const;
                bool operator>(const dbstring& dbstr) const;

                //Conversions
                inline operator const char*() const { return char_seq; }
                inline operator std::string() const { return std::string(char_seq);}
                inline const char* c_str() const { return char_seq; }
                inline std::string std_str() const { return std::string(char_seq);}

                friend std::ostream& operator<< (std::ostream& os, const dbstring& dbstr)
                {
                    os << dbstr.char_seq;
                    return os;
                }

                //Hashing support
                std::size_t hash_value() const;

                //
                size_t size() const;
        };

        class dbstring_iterator_state;
        class dbstring_iterator
        {
            private:
                std::shared_ptr<dbstring_iterator_state> state;
            public:
                dbstring_iterator(std::shared_ptr<dbstring_iterator_state> state_in) { state = state_in;}
                ~dbstring_iterator() {}
                bool operator==(const dbstring_iterator& other);
                bool operator!=(const dbstring_iterator& other);
                dbstring_iterator& operator++();
                sharedobj_ptr<dbstring>operator*();
        };

//@brief    Allocates and constructs and object of type @a T and returns a
//object of type @a sharedobj_ptr<T>.
//template <class... Args> sharedobj_ptr<benedias::memdb::dbstring> make_sharedobj(Args&&... args);
sharedobj_ptr<benedias::memdb::dbstring> make_sharedobj(const char*);
    } // namespace memdb

} // namespace benedias

namespace std
{
    template <>
    struct hash<benedias::memdb::dbstring>
    {
        size_t operator()(benedias::memdb::dbstring const& dbs) const
        {
            return dbs.hash_value();
        }
    };
} //namespace std

#endif // BENEDIAS_DB_STRING_H_INCLUDED
