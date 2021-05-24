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
#include "xpack/json.h"

using namespace std;

struct Example {
    string type;
    xpack::JsonData data;
    XPACK(O(type, data));
};

struct Range {
    int min;
    int max;
    XPACK(O(min, max));
};

struct User {
    int id;
    string name;
    XPACK(O(id, name));
};

static void test(const std::string &data) {
    Example e;
    xpack::json::decode(data, e);
    if (e.type == "range") {
        Range r;
        e.data.decode(r);
        cout<<"range. min:"<<r.min<<", max:"<<r.max<<endl;
        cout<<e.data["min"].GetInt()<<endl;
        cout<<"raw:"<<e.data.String()<<endl;
    } else if (e.type == "user") {
        User u;
        e.data.decode(u);
        cout<<"user. id:"<<u.id<<", name:"<<u.name<<endl;
        cout<<"raw:"<<e.data.String()<<endl;
    } else {
        cout<<"unknow type"<<endl;
    }
    cout<<"re encode:"<<xpack::json::encode(e)<<endl;
}


int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    string s1 = "{\"type\":\"range\", \"data\":{\"min\":12, \"max\":22}}";
    string s2 = "{\"type\":\"user\", \"data\":{\"id\":123, \"name\":\"xpack\"}}";
    test(s1);
    test(s2);

    return 0;
}
