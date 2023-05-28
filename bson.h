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

#ifndef __X_PACK_BSON_H
#define __X_PACK_BSON_H

#include "bson_decoder.h"
#include "bson_encoder.h"
#include "xpack.h"

#ifdef X_PACK_SUPPORT_CXX0X // support c++11 or later
#include "bson_builder.h"
#endif


namespace xpack {

class bson {
public:
    template <class T>
    static void decode(const std::string &data, T &val) {
        BsonDecoder de;
        de.decode(data, val);
    }

    template <class T>
    static void decode(const uint8_t* data, size_t len, T &val) {
        BsonDecoder de;
        de.decode(data, len, val);
    }

    template <class T>
    static std::string encode(const T &val) {
        BsonEncoder en;
        return en.encode(val);
    }
};

}

#endif
