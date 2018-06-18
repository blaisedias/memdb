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
#include <algorithm>
#include "dbstring.h"

using benedias::memdb::dbstring;
using benedias::memdb::dbstring_iterator;
#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
using benedias::memdb::dbstring_domain;
#endif
using benedias::sharedobj_ptr;
using benedias::make_sharedobj;

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


#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
void populate_domain(const char** teststrs, unsigned num, std::shared_ptr<dbstring_domain>dom)
{
    std::cout  << std::endl << "populate_domain {" << std::endl;
    std::vector<sharedobj_ptr<dbstring>> v;
    for (unsigned i = 0; i < num; i++)
    {
        v.push_back(dbstring::make_sharedobj(teststrs[i], dom));
    }

    for(auto it = v.cbegin(); it != v.end(); ++it)
    {
        std::cout << (*it).self() << " " << (*it)->std_str() << " " << (*it)->size() << std::endl;
    }
    std::cout  << "} populate_domain" << std::endl;
}

void iterate_domain(std::shared_ptr<dbstring_domain>dom)
{
    {
        std::cout  << std::endl << "iterate_domain {" << std::endl;
        for(dbstring_iterator itr = dbstring::begin(dom);
                itr != dbstring::end(); ++itr)
        {
            std::cout << (*itr)->c_str() << std::endl;
        }
        std::cout << "} iterate_domain" << std::endl << std::endl;
    }
}
#else
void populate0(const char** teststrs, unsigned num)
{
    std::cout  << std::endl << "populate {" << std::endl;
    std::vector<sharedobj_ptr<dbstring>> v;
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
#endif

int main(int argc, char* argv[] )
{
    std::setlocale(LC_ALL, "en_US.UTF-8");
#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
    std::shared_ptr<dbstring_domain> dom1 = dbstring::get_domain();
    std::shared_ptr<dbstring_domain> dom2 = dbstring::get_domain();
    std::shared_ptr<dbstring_domain> dom3 = dbstring::get_domain();
    populate_domain(strs1, sizeof(strs1)/sizeof(*strs1), dom1);
    iterate_domain(dom1);
    populate_domain(strs2, sizeof(strs2)/sizeof(*strs2), dom2);
    iterate_domain(dom2);
    populate_domain(strs3, sizeof(strs3)/sizeof(*strs3), dom3);
    iterate_domain(dom3);
#else
    populate0(strs1, sizeof(strs1)/sizeof(*strs1));
    populate0(strs2, sizeof(strs2)/sizeof(*strs2));
    populate0(strs3, sizeof(strs3)/sizeof(*strs3));
    iterate0();
#endif
    std::vector<sharedobj_ptr<dbstring>> v;
    for(dbstring_iterator itr = dbstring::begin();
            itr != dbstring::end(); ++itr)
    {
        v.push_back(*itr);
    }
    std::sort(v.begin(), v.end());
    for(auto it = v.cbegin(); it != v.end(); ++it)
    {
        std::cout << (*it).self() << " " << (*it)->std_str() << " " << (*it)->size() << std::endl;
    }
    std::cout << sizeof(v[0].get()) << std::endl;
    std::cout << sizeof(*v[0].get()) << std::endl;
    auto sd = benedias::memdb::make_sharedobj("alpha");
}

