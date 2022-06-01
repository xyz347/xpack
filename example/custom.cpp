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

struct Test {
    unsigned int uid;
    string  name;
    XPACK(C(hexint, F(0), uid), O(name));
};

namespace xpack {

template <class OBJ>
bool hexint_decode( OBJ&obj, const char *name, unsigned int&data, const xpack::Extend *ext) {
    string raw;
    bool ret = obj.decode(name, raw, ext);
    if (ret) {
        sscanf(raw.c_str(), "%x", &data);
    }
    return ret;
}

template <class OBJ>
bool hexint_encode(OBJ&obj, const char *name, const unsigned &data, const xpack::Extend *ext) {
    char buf[64];
    sprintf(buf, "%x", data);
    bool ret = obj.encode(name, buf, ext);
    return ret;
}
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    Test t;
    t.uid = 127;
    t.name = "PonyJack";

    string str = xpack::json::encode(t);
    cout<<str<<endl;

    Test n;
    xpack::json::decode(str, t);
    cout<<t.uid<<endl;
    return 0;
}
