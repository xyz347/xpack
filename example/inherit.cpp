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

struct P1 {
    string mail;
    XPACK(O(mail));
};

struct P2 {
    long version;
    XPACK(O(version));
};

struct Test:public P1, public P2 {
    long uid;
    string  name;
    XPACK(I(P1, P2), A(uid, "id"), O(name));
};

struct Pa {
    string mail;
    XPACK(O(mail));
};

struct Pb: public Pa {
    long version;
    XPACK(I(Pa), O(version));
};

struct Pc:public Pb {
    long uid;
    string  name;
    XPACK(I(Pb), A(uid, "id"), O(name));
};

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    Test t;
    string json="{\"mail\":\"pony@xpack.com\", \"version\":2019, \"id\":123, \"name\":\"Pony\"}";

    xpack::json::decode(json, t);
    cout<<xpack::json::encode(t)<<endl;

    Pc p;
    xpack::json::decode(json, p);
    cout<<xpack::json::encode(p)<<endl;

    return 0;
}
