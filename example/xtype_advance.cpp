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

// The requirement is to encode seq1 when type==1, and encode seq2 when type==2
struct Sub {
    int    type;
    string seq1;
    string seq2;
    XPACK(O(type, seq1, seq2));
};


struct Test {
    unsigned int uid;
    string  name;
    Sub     sub;
    XPACK(O(uid, name, sub));
};

namespace xpack {

template<>
struct is_xpack_xtype<Sub> {static bool const value = true;};

template <class Decoder>
bool xpack_xtype_decode(Decoder& de, Sub &data, const xpack::Extend *ext) {
    de.decode_struct(data, ext);
    return true;
}

template <class BaseEncoder>
class MyEncoder {
public:
    MyEncoder(BaseEncoder&b, const Sub&sub):_b(&b), _s(&sub){}
    template <class T>
    bool encode(const char*key, const T&val, const xpack::Extend *ext) {
        if (strcmp(key, "seq1") == 0) {
            if (_s->type == 1) {
                return _b->encode(key, val, ext);
            } else {
                return false;
            }
        } else if (strcmp(key, "seq2") == 0) {
            if (_s->type == 2) {
                return _b->encode(key, val, ext);
            } else {
                return false;
            }
        } else {
            return _b->encode(key, val, ext);
        }
    }
private:
    BaseEncoder *_b;
    const Sub *_s;
};

template <class Encoder>
bool xpack_xtype_encode(Encoder &en, const char *name, const Sub &data, const xpack::Extend *ext) {
    (void)name;
    MyEncoder<Encoder> my(en, data);
    data.__x_pack_encode(my, data, ext);
    return true;
}

}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    Test t;
    t.uid = 127;
    t.name = "PonyJack";
    t.sub.type = 1;
    t.sub.seq1 = "10086";
    t.sub.seq2 = "10001";

    string str = xpack::json::encode(t);
    cout<<str<<endl;

    Test n;
    xpack::json::decode(str, t);
    cout<<t.uid<<endl;

    t.sub.type = 2;
    cout<<xpack::json::encode(t)<<endl;

    return 0;
}
