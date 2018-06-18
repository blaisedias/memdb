/*

Copyright (C) 2014,2015  Blaise Dias

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
*/
#include <cstdio>
#include <cstring>
#include <clocale>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <stdlib.h>
#include <memory>
#include <fstream>
#include <boost/serialization/vector.hpp>

#include "dbstring.h"
#include "dbstring_boost_serialization.h"
#include "stringlist.hpp"

using benedias::memdb::dbstring;
using benedias::memdb::dbstring_iterator;
using benedias::sharedobj_ptr;

const char* strs1[] = {
    "pqr",
    "stu",
    "vwx",
    "abc",
    "def",
    "ghi",
    "jkl",
    "mno",
    "yz"
};

const char* strs2[] = {
    "def",
    "jkl",
    "pqr",
    "vwx",
};

const char* strs3[] = {
    "abc",
    "ghi",
    "mno",
    "stu",
    "yz"
};

void populate0(const char** teststrs, unsigned num, std::vector<sharedobj_ptr<dbstring>>& v)
{
    std::cout  << std::endl << "populate {" << std::endl;
    for (unsigned i = 0; i < num; i++)
    {
        v.push_back(dbstring::make_sharedobj(teststrs[i]));
    }

    for(auto it = v.cbegin(); it != v.end(); ++it)
    {
        std::cout << (*it).self() << " " << (*it)->std_str() << " " << (*it)->size() << std::endl;
    }
    std::cout  << "} populate" << std::endl;
}

void iterate0()
{
    {
        std::cout  << std::endl << "iterate {" << std::endl;
        for(dbstring_iterator itr = dbstring::begin();
                itr != dbstring::end(); ++itr)
        {
            std::cout << (*itr)->c_str() << std::endl;
        }
        std::cout << "} iterate" << std::endl << std::endl;
    }
}

void dump(std::vector<sharedobj_ptr<dbstring>>& v)
{
    std::cout  << std::endl << "dump {" << std::endl;
    for(auto it = v.cbegin(); it != v.end(); ++it)
    {
        std::cout << (*it).self() << " " << (*it)->std_str() << " " << (*it)->size() << std::endl;
    }
    std::cout  << "} dump" << std::endl;
}

int main(int argc, char* argv[] )
{
    {
#if 0
        std::vector<sharedobj_ptr<dbstring>> vdbs;
        populate0(strs1, sizeof(strs1)/sizeof(*strs1), vdbs);
        dump(vdbs);
#else
        auto spdbs = dbstring::make_sharedobj("ZZZ");
        std::cout << "hash(spdbs)   " << std::hash<sharedobj_ptr<dbstring>>()(spdbs) << std::endl;
        dbstring& rdbs = *(spdbs.get());
        std::cout << "rdbs   " << rdbs << std::endl;
        std::cout << "spdbs.get()   " << spdbs.get() << std::endl;
        std::cout << "spdbs->c_str()   " << spdbs->c_str() << std::endl;
        const char* cstr = rdbs;
        std::string stdstr = rdbs;
        std::cout << "cstr   " << cstr << std::endl;
        std::cout << "stdstr   " << stdstr << std::endl;
#endif

    }
    std::cout << "------------------------" << std::endl;
    std::cout << "------------------------" << std::endl;
    {
    }
    std::cout << "------------------------" << std::endl;
    std::cout << "All Done. " << std::endl;
}

