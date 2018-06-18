/*

Copyright (C) 2017,2018  Blaise Dias

This is free software: you can redistribute it and/or modify
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
#ifndef BENEDIAS_DB_STRING_BOOST_SERIALIZATION_H_INCLUDED
#define BENEDIAS_DB_STRING_BOOST_SERIALIZATION_H_INCLUDED

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

BOOST_SERIALIZATION_SPLIT_FREE(benedias::sharedobj_ptr<benedias::memdb::dbstring>)
namespace boost {
namespace serialization {

void save(boost::archive::text_oarchive& ar,
        const benedias::sharedobj_ptr<benedias::memdb::dbstring>& spdbs,
        const boost::serialization::version_type& version)
{
    std::string str = spdbs->std_str();
    ar & str;
}

void load(boost::archive::text_iarchive& ar,
        benedias::sharedobj_ptr<benedias::memdb::dbstring>& spdbs,
        const boost::serialization::version_type& version)
{
    std::string tmp;
    ar & tmp;

    benedias::sharedobj_ptr<benedias::memdb::dbstring> sp = benedias::memdb::dbstring::make_sharedobj(tmp);
    spdbs = sp;
}

} // namespace serialization
} // namespace boost

#endif
