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

#ifndef __X_PACK_JSON_H
#define __X_PACK_JSON_H

#include "json_decoder.h"
#include "json_encoder.h"
#include "xpack.h"

namespace xpack {

class json {
public:
    template <class T>
    static void decode(const std::string &data, T &val) {
        JsonDecoder doc(data);
        doc.decode(NULL, val, NULL);
    }
    template <class T>
    static void decode_file(const std::string &file_name, T &val) {
        JsonDecoder doc(file_name, true);
        doc.decode(NULL, val, NULL);
    }

    template <class T>
    static std::string encode(const T &val) {
        JsonEncoder doc(-1);
        doc.encode(NULL, val, NULL);
        return doc.String();
    }

    template <class T>
    static std::string encode(const T &val, int flag, int indentCount, char indentChar) {
        JsonEncoder doc(indentCount, indentChar);
        Extend ext(flag, NULL);

        doc.encode(NULL, val, &ext);
        return doc.String();
    }
};

}

#endif