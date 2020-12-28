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


#ifndef __X_PACK_EXTEND_H
#define __X_PACK_EXTEND_H

#include <cstddef>

#include <map>
#include "config.h"
#include "util.h"

namespace xpack {

// user flag
#define X_PACK_FLAG_0 0
#define X_PACK_FLAG_OE (1<<0) // omitempty, in encode
#define X_PACK_FLAG_M  (1<<1) // mandatory, in decode

#define X_PACK_FLAG_ATTR (1<<15) // for xml encode, encode in attribute

// control flag
#define X_PACK_CTRL_FLAG_INHERIT (1<<0)

// Alias name. [def ][type:name[,flag]]  flag not support any more
struct Alias {
    const char *raw;        // raw name
    const char *alias;        // alias define

    Alias(const char *_raw, const char *_alias):raw(_raw), alias(_alias) {
        std::vector<std::string> l1;
        Util::split(l1, alias, ' ');

        for (size_t i=0; i<l1.size(); ++i) {
            std::vector<std::string> l2;
            Util::split(l2, l1[i], ':');

            std::vector<std::string> l3;
            Util::split(l3, l2[l2.size()-1], ',');

            if (l2.size() == 2) {
                types[l2[0]] = l3[0];
            } else if (l2.size()==1) {
                def = l3[0];
            } else {
                // error format
            }
        }
    }

    const char *Name(const char *type) const {
        std::map<std::string, std::string>::const_iterator iter = types.find(type);
        if (iter == types.end()) {
            if (def.empty()) {
                return raw;
            } else {
                return def.c_str();
            }
        } else {
            return iter->second.c_str();
        }
    }
private:
    std::string def;
    std::map<std::string, std::string> types; // not support flag now
};

struct Extend {
    int flag;
    int ctrl_flag;
    const Alias *alias;

    Extend(int _flag, const Alias *_alias):flag(_flag), alias(_alias), ctrl_flag(0) {
    }

    Extend(const Extend *ext) {
        if (NULL != ext) {
            flag = ext->flag;
            ctrl_flag = ext->ctrl_flag;
            alias = ext->alias;
        } else {
            flag = 0;
            ctrl_flag = 0;
            alias = NULL;
        }
    }

    static int Flag(const Extend *ext) {
        if (NULL == ext) {
            return 0;
        } else {
            return ext->flag;
        }
    }

    static int CtrlFlag(const Extend *ext) {
        if (NULL == ext) {
            return 0;
        } else {
            return ext->ctrl_flag;
        }
    }


    static bool OmitEmpty(const Extend *ext) {
        return NULL!=ext && (ext->flag&X_PACK_FLAG_OE);
    }
    static bool Mandatory(const Extend *ext) {
        return NULL!=ext && (ext->flag&X_PACK_FLAG_M);
    }
    static bool Attribute(const Extend *ext) {
        return NULL!=ext && (ext->flag&X_PACK_FLAG_ATTR);
    }
};

}

#endif

