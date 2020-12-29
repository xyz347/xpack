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

// #include <sys/time.h> linux only
#include <iostream>
#include "xpack/json.h"
#ifndef _MSC_VER
#include <sys/time.h>
#endif

using namespace std;

#ifdef _MSC_VER
struct timeval {
    long tv_sec;
    long tv_usec;
};
#endif

// timeval is thirdparty struct
XPACK_OUT(timeval, O(tv_sec, tv_usec));

struct T {
    int  a;
    string b;
    timeval t;
    XPACK(O(a, b, t));
};


int main(int argc, char *argv[]) {
    T t;
    T r;
    t.a = 123;
    t.b = "xpack";
    t.t.tv_sec = 888;
    t.t.tv_usec = 999;
    string s = xpack::json::encode(t);
    cout<<s<<endl;
    xpack::json::decode(s, r);
    cout<<r.a<<','<<r.b<<','<<r.t.tv_sec<<','<<r.t.tv_usec<<endl;
    return 0;
}
