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
#include "xpack/json.h" // 包含这个头文件

using namespace std;

struct User {
    int64_t id;
    string  name;
    string  mail;
    User(int64_t i=0, const string& n="", const string& m=""):id(i),name(n),mail(m){}
    XPACK(O(id, name, mail)); // 添加宏定义XPACK在结构体定义结尾
};

struct Group {
    string  name;
    int64_t master;
    vector<User> members;
    XPACK(O(name, master, members)); // 添加宏定义XPACK在结构体定义结尾
};

int main(int argc, char *argv[]) {
    Group g;
    g.name = "C++";
    g.master = 2019;
    g.members.resize(2);
    g.members[0] = User(1, "Jack", "jack@xpack.com");
    g.members[1] = User(2, "Pony", "pony@xpack.com");

    string json = xpack::json::encode(g);      // 结构体转json
    cout<<json<<endl;

    Group n;
    xpack::json::decode(json, n);             // json转结构体
    cout<<n.name<<endl;

    vector<int> vi;
    xpack::json::decode("[1,2,3]", vi);     // 直接转换vector
    cout<<vi.size()<<','<<vi[1]<<endl;

    map<string, int> m;
    xpack::json::decode("{\"1\":10, \"2\":20}", m); // 直接转换map
    cout<<m.size()<<','<<m["2"]<<endl;

    return 0;
}