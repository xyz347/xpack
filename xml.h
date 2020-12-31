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

#ifndef __X_PACK_XML_H
#define __X_PACK_XML_H

#include "xml_decoder.h"
#include "xml_encoder.h"
#include "xpack.h"

namespace xpack {

class xml {
public:
    template <class T>
    static void decode(const std::string &data, T &val) {
        XmlDecoder doc(data);
        doc.decode(NULL, val, NULL);
    }
    template <class T>
    static void decode_file(const std::string &file_name, T &val) {
        XmlDecoder doc(file_name, true);
        doc.decode(NULL, val, NULL);
    }

    template <class T>
    static std::string encode(const T &val, const std::string&root) {
        XmlEncoder doc(-1);
        doc.encode(root.c_str(), val, NULL);
        return doc.String();
    }

    template <class T>
    static std::string encode(const T &val, const std::string&root, int flag, int indentCount, char indentChar) {
        XmlEncoder doc(indentCount, indentChar);
        Extend ext(flag, NULL);

        doc.encode(root.c_str(), val, &ext);
        return doc.String();
    }
};

}

#endif