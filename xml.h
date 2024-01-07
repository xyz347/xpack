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
/*
for xml like
<user>
    <name>Pony</name>
    <age>20</age>
</user>

Just define such a structure
struct User {
    string name;
    int age;
    XPACK(O(name, age));
};

!!!!NO NEED!!! to define:
struct Data {
    User user;
    XPACK(O(user));
};

xml::decode will skip the root node, so for the xml defined above,
it only needs to be decode like this:
User u;
xpack::xml::decode(str, u);
*/

class xml {
public:
    template <class T>
    static void decode(const std::string &data, T &val) {
        XmlDecoder de;
        de.decode(data, val);
    }
    template <class T>
    static void decode_file(const std::string &file_name, T &val) {
        XmlDecoder de;
        de.decode_file(file_name, val);
    }
    template <class T>
    static std::string encode(const T &val, const std::string&root) {
        XmlEncoder en;
        return en.encode(val, root);
    }

    template <class T>
    static std::string encode(const T &val, const std::string&root, int flag, int indentCount, char indentChar) {
        (void)flag;
        XmlEncoder en(indentCount, indentChar);
        return en.encode(val, root);
    }
};

}

#endif