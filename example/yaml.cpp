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
#include "xpack/yaml.h"
#include "xpack/json.h"

using namespace std;

struct User {
    int64_t id;
    string  name;
    string  mail;
    User(int64_t i=0, const string& n="", const string& m=""):id(i),name(n),mail(m){}
    XPACK(O(id, name, mail));
};

struct Group {
    string  name;
    int64_t master;
    map<string, string> flags;
    vector<User> members;
    User         arrays[2];
    XPACK(O(name, master, flags, members, arrays));
};

int main() {
    User u;
    xpack::yaml::decode_file("./test.yml", u);
    string json = xpack::json::encode(u);
    cout<<"json:"<<json<<endl;
    cout<<"yaml is =================="<<endl<<xpack::yaml::encode(u)<<endl<<"=================="<<endl;

    Group g;
    g.name = "C++";
    g.master = 2019;
    g.members.resize(2);
    g.members[0] = User(1, "Jack", "jack@xpack.com");
    g.members[1] = User(2, "Pony", "pony@xpack.com");
    g.arrays[0] = g.members[0];
    g.arrays[1] = g.members[1];
    g.flags["a"] = "good";
    g.flags["b"] = "nice";
    string ys = xpack::yaml::encode(g);
    cout<<"yaml is =================="<<endl<<ys<<endl<<"=================="<<endl;

    Group g1;
    xpack::yaml::decode(ys, g1);
    cout<<xpack::json::encode(g1)<<endl;

    return 0;
}
