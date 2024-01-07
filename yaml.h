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

#ifndef __X_PACK_YAML_H
#define __X_PACK_YAML_H

#include "yaml_decoder.h"
#include "yaml_encoder.h"
#include "xpack.h"

namespace xpack {

class yaml {
public:
    template <class T>
    static void decode(const std::string &data, T &val) {
        YamlDecoder de;
        de.decode(data, val);
    }
    template <class T>
    static void decode_file(const std::string &file_name, T &val) {
        YamlDecoder de;
        de.decode_file(file_name, val);
    }

    template <class T>
    static std::string encode(const T &val) {
        YamlEncoder en;
        return en.encode(val);
    }
};

}

#endif
