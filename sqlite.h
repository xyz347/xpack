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

#ifndef __X_PACK_SQLITE_H
#define __X_PACK_SQLITE_H

#include "sqlite_decoder.h"
#include "xpack.h"

namespace xpack {

class sqlite {
public:
    // convert result of sqlite_get_table to a struct or vector<struct>
    template <class T>
    static void decode(const char **result, int rows, int cols, T &val) {
        SQLiteDecoder de(result, rows, cols);
        de.decode_top(val, NULL);
    }

    // select name from test where id = 1; // just want to get name, did not want to use a struct
    template <class T>
    static void decode(const char **result, int rows, int cols, const std::string&field, T &val) {
        SQLiteDecoder de(result, rows, cols);
        de.decode_column(field.c_str(), val, NULL);
    }
};

}

#endif