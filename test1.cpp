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
#include "sharedobj.h"
#include "dbstring.h"

using benedias::memdb::dbstring;
using benedias::memdb::dbstring_iterator;
using benedias::sharedobj_ptr;

const char* teststrs[] = {
    "abc",
    "def",
    "ghi",
    "abc",
    "def",
    "ghi"
};

void sub_test()
{
    std::vector<sharedobj_ptr<dbstring>> v;
    for (unsigned i = 0; i < sizeof(teststrs)/sizeof(*teststrs); i++)
    {
        v.push_back(dbstring::make_sharedobj(teststrs[i]));
    }

    for(auto it = v.cbegin(); it != v.end(); ++it)
    {
        std::cout << (*it).self() << " " << (*it)->std_str() << " " << (*it)->size() << std::endl;
    }
}

void test()
{
    static unsigned invoke_count = 0;
    std::cout << "test() {" << std::endl;
    ++invoke_count;
    sub_test();
    if (invoke_count == 2)
    {
        std::cout  << std::endl << "Reap {" << std::endl;
        dbstring::reap();
        std::cout << "} Reap" << std::endl << std::endl;
    }
    if (invoke_count == 3)
    {
        std::cout  << std::endl << "iterate {" << std::endl;
        for(dbstring_iterator itr = dbstring::begin();
                itr != dbstring::end(); ++itr)
        {
            std::cout  <<  (*itr)->c_str() << std::endl;
        }
        std::cout << "} iterate" << std::endl << std::endl;
    }
    std::cout << "} test()" << std::endl;
}

void test0()
{
    std::cout << "test0() {" << std::endl;
    std::cout << "auto dbs = dbstring::make_sharedobj(\"abc\");" << std::endl;
    auto dbs = dbstring::make_sharedobj("abc");
    std::cout << "} test0()" << std::endl;
}

void test1()
{
    std::vector<sharedobj_ptr<dbstring>> v;
    std::cout << "test1() {" << std::endl;
    std::cout << "v.push_back(dbstring::make_sharedobj(\"abc\"));" << std::endl;
    v.push_back(dbstring::make_sharedobj("abc"));
    std::cout << "} test1()" << std::endl;
}

void test2()
{
    std::vector<sharedobj_ptr<dbstring>> v;
    std::cout << "test2() {" << std::endl;
    std::cout << "v.emplace_back(dbstring::make_sharedobj(\"abc\"));" << std::endl;
    v.emplace_back(dbstring::make_sharedobj("abc"));
    std::cout << "} test2()" << std::endl;
}

void test3()
{
    std::vector<sharedobj_ptr<dbstring>> v;
    std::cout << "test3() {" << std::endl;
    std::cout << "auto dbs = dbstring::make_sharedobj(\"abc\");" << std::endl;
    auto dbs = dbstring::make_sharedobj("abc");
    std::cout << "v.push_back(dbs);" << std::endl;
    v.push_back(dbs);
    std::cout << "} test3()" << std::endl;
}

void test4()
{
    std::vector<sharedobj_ptr<dbstring>> v;
    std::cout << "test4() {" << std::endl;
    std::cout << "auto dbs = dbstring::make_sharedobj(\"abc\");" << std::endl;
    auto dbs = dbstring::make_sharedobj("abc");
    std::cout << "v.emplace_back(dbs);" << std::endl;
    v.emplace_back(dbs);
    std::cout << "} test4()" << std::endl;
}

void test5()
{
    std::cout << "test5() {" << std::endl;
    sub_test();
    {
        std::cout  << std::endl << "iterate {" << std::endl;
        for(dbstring_iterator itr = dbstring::begin();
                itr != dbstring::end(); ++itr)
        {
            std::cout << (*itr)->c_str() << std::endl;
        }
        std::cout << "} iterate" << std::endl << std::endl;
    }
    std::cout << "} test5()" << std::endl;
}

int main(int argc, char* argv[] )
{
    int loopcount = 3;
    std::setlocale(LC_ALL, "en_US.UTF-8");
    void (*tf)() = test;
    if (argc > 1)
    {
        switch(*argv[1])
        {
            case '0':
                tf = test0; break;
            case '1':
                tf = test1; break;
            case '2':
                tf = test2; break;
            case '3':
                tf = test3; break;
            case '4':
                tf = test4; break;
            case '5':
                loopcount = 1;
                tf = test5; break;

        }
    }
    while(loopcount--)
    {
        tf();
        std::cout << "-----------------------" << std::endl<< std::endl;
    }

    auto dbs = dbstring::make_sharedobj("");
    std::cout << " sizeof(sharedobj_ptr<dbstring>)= " << sizeof(dbs) << std::endl;

#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
    auto dom = dbstring::get_domain();
    std::cout << " sizeof(shared_ptr<dbstring_domain>)= " << sizeof(dom) << std::endl<< std::endl;
#endif


    std::cout << "All Done. " << std::endl;
}

