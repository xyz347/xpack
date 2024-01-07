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

#include <stdio.h>
#include <iostream>
#include "xpack/json.h"

using namespace std;

struct Base {
    int a;
    int b;
    XPACK(O(a, b));
};

struct Data {
    int type;
    union {
        Base  b;
        int   i;
        char  s[10];
    };
    string email;
    string birthday;
    // 'type' determines which member of the union to use, so we put type in 'C'
    // union does not need to be put into XPACK, we will handled it in 'type'
    XPACK(C(data_uinon, F(0), type), O(email, birthday)); 
};

struct User {
    string name;
    Data   data;
    XPACK(O(name, data));
};

namespace xpack { // must define in namespace xpack

template<class OBJ>
bool data_uinon_decode(OBJ &obj, Data &d, const char*key, int &type, const Extend *ext) {
    (void)ext;
    obj.decode(key, type, ext);
    switch (type) {
        case 1:
            obj.decode("b", d.b, NULL); // key("b") should same as encode
            break;
        case 2:
            obj.decode("i", d.i, NULL);
            break;
        case 3:
            obj.decode("s", d.s, NULL);
            break;
    }
    return true;
}

template<class OBJ>
bool data_uinon_encode(OBJ &obj, const Data &d, const char*key, const int &type, const Extend *ext) {
    (void)ext;
    obj.encode(key, type, NULL);
    switch (type) {
        case 1:
            obj.encode("b", d.b, NULL);
            break;
        case 2:
            obj.encode("i", d.i, NULL);
            break;
        case 3:
            obj.encode("s", d.s, NULL);
            break;
    }
    return true;
}

}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    User u;
    u.name = "Pony";
    u.data.type = 1;
    u.data.b.a = 10;
    u.data.b.b = 12;
    u.data.email = "Pony@xpack.com";
    u.data.birthday = "20200202";

    string s = xpack::json::encode(u);
    cout<<s<<endl;

    u.data.type = 2;
    u.data.i = 22;
    s = xpack::json::encode(u);
    cout<<s<<endl;

    u.data.type = 3;
    strcpy(u.data.s, "hello");
    s = xpack::json::encode(u);
    cout<<s<<endl;

    User u1;
    xpack::json::decode(s, u1);
    cout<<u1.data.s<<endl;
    return 0;
}
