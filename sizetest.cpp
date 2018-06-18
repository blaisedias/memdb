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
#include "dbstring.h"

using benedias::memdb::dbstring;
using benedias::memdb::dbstring_iterator;
using benedias::sharedobj_ptr;

int main(int argc, char* argv[] )
{
    std::shared_ptr<std::string> spstds = std::make_shared<std::string>("abc");
    auto spdbs = dbstring::make_sharedobj("abc");
    std::cout << " sizeof(shared_ptr<std::string>)= " << sizeof(spstds) << std::endl;
    std::cout << " sizeof(sharedobj_ptr<dbstring>)= " << sizeof(spdbs) << std::endl;
    std::cout << " sizeof(std::string)= " << sizeof(*spstds) << std::endl;
    std::cout << " sizeof(dbstring)= " << sizeof(*spdbs) << std::endl;

#ifdef  BENEDIAS_USE_DBSTRING_DOMAINS
    auto dom = dbstring::get_domain();
    std::cout << " sizeof(shared_ptr<dbstring_domain>)= " << sizeof(dom) << std::endl<< std::endl;
#endif


    std::cout << "All Done. " << std::endl;
}

