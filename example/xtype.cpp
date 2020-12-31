/*
* Copyright (C) 2021 Duowan Inc. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <iostream>
#include "xpack/xtype.h"
#include "xpack/json.h"
#include "xpack/xml.h"

using namespace std;

struct Date {
    long unix_time;
};

namespace xpack { // must define in namespace xpack

template<>
struct is_xpack_xtype<Date> {static bool const value = true;};

// implement decode
template<class OBJ>
bool xpack_xtype_decode(OBJ &obj, const char*key, Date &val, const Extend *ext) {
    std::string str;
    obj.decode(key, str, ext);
    if (str.empty()) {
        return false;
    }

#ifndef _MSC_VER
    tm ttm;

    if (0 != strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &ttm)) {
        val.unix_time = mktime(&ttm);
    } else {
        val.unix_time = 0;
    }
#else
    static int days[]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 32};
    struct tm ttm={0};
    sscanf_s(str.c_str(), "%d-%d-%d %d:%d:%d", &ttm.tm_year, &ttm.tm_mon, &ttm.tm_mday, &ttm.tm_hour, &ttm.tm_min, &ttm.tm_sec);
    ttm.tm_mon-=1; // mon[0-11]
    ttm.tm_year-=1900; // since 1900
    val.unix_time = mktime(&ttm);
#endif
    return true;
}

// implement encode
template<class OBJ>
bool xpack_xtype_encode(OBJ &obj, const char*key, const Date &val, const Extend *ext) {
    time_t tt = (time_t)val.unix_time;
    tm     ttm;

#ifndef _MSC_VER
    localtime_r(&tt, &ttm);
#else
    localtime_s(&ttm, &tt);
#endif

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ttm);
    return obj.encode(key, buf, ext);
}

}

struct User {
    int64_t id;
    string  name;
    string  mail;
    Date    d;
    User(int64_t i=0, const string& n="", const string& m="", long _d=0):id(i),name(n),mail(m){d.unix_time = _d;}
    XPACK(O(id, name, mail, d));
};

struct Group {
    string  name;
    int64_t master;
    vector<User> members;
    XPACK(O(name, master, members));
};

int main(int argc, char *argv[]) {
    Group g;
    g.name = "C++";
    g.master = 2019;
    g.members.resize(2);
    g.members[0] = User(1, "Jack", "jack@xpack.com", 1);
    g.members[1] = User(2, "Pony", "pony@xpack.com", 1609249232);

    string json = xpack::json::encode(g, 0, 2, ' ');
    cout<<json<<endl;

    string xml = xpack::xml::encode(g, "root", 0, 2, ' ');
    cout<<xml<<endl;

    Group n1;
    xpack::json::decode(json, n1);
    cout<<n1.name<<';'<<n1.members[0].name<<';'<<n1.members[0].d.unix_time<<endl;

    Group n2;
    xpack::xml::decode(xml, n2);
    cout<<n2.name<<';'<<n2.members[1].name<<';'<<n2.members[1].d.unix_time<<endl;

    return 0;
}
