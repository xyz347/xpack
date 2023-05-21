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

enum Enum {
    X = 0,
    Y = 1,
    Z = 2,
};

enum class Enum1 {
    X = 0,
    Y = 1,
    Z = 2,
};

enum class Enum2:int64_t {
    X = 0,
    Y = 1,
    Z = 2,
};

struct Test {
    string  name;
    Enum    e;
    Enum1   e1;
    Enum2   e2;
    XPACK(O(name), E(F(0), e), O(e1, e2));
};

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    Test t;
    string json="{\"name\":\"IPv4\", \"e\":1, \"e1\":2, \"e2\":2}";

    xpack::json::decode(json, t);
    cout<<xpack::json::encode(t)<<endl;
    return 0;
}
