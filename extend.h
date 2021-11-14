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
#include <set>
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

// Alias name. [def ][type:name[,flag,key@value,flag]]  def not support flag
struct Alias {
    const char *raw;        // raw name
    const char *alias;      // alias define

    struct Type {
        std::string name;
        std::set<std::string> flags;
        std::map<std::string, std::string> kv_flags;
    };

    Alias(const char *_raw, const char *_alias):raw(_raw), alias(_alias) {
        std::vector<std::string> tps;
        Util::split(tps, alias, ' ');

        for (size_t i=0; i<tps.size(); ++i) {
            std::vector<std::string> typeFlag;
            Util::split(typeFlag, tps[i], ':', 1);

            if (typeFlag.size() == 1) { // no ':', default name
                def = tps[i];
            } else { // type:name[,flags]
                Type tp;

                std::vector<std::string> nameFlags;
                Util::split(nameFlags, typeFlag[1], ',');
                tp.name = nameFlags[0];
                if (nameFlags.size() > 1) {
                    for (size_t j=1; j<nameFlags.size(); ++j) {
                        std::vector<std::string> kvFlag;
                        Util::split(kvFlag, nameFlags[j], '@', 1);
                        if (kvFlag.size() == 1) {
                            tp.flags.insert(nameFlags[j]);
                        } else {
                            tp.kv_flags[kvFlag[0]] = kvFlag[1];
                        }
                    }
                }

                types[typeFlag[0]] = tp;
            }
        }
    }

    const char *Name(const char *type) const {
        std::map<std::string, Type>::const_iterator iter = types.find(type);
        if (iter == types.end()) {
            if (def.empty()) {
                return raw;
            } else {
                return def.c_str();
            }
        } else {
            return iter->second.name.c_str();
        }
    }

    std::string Flag(const std::string&type, const std::string& flag) const {
        std::map<std::string, Type>::const_iterator it1 = types.find(type);
        if (it1 != types.end()) {
            if (it1->second.flags.find(flag) != it1->second.flags.end()) {
                return flag;
            }
            std::map<std::string, std::string>::const_iterator it2 = it1->second.kv_flags.find(flag);
            if (it2 != it1->second.kv_flags.end()) {
                return it2->second;
            }
        }
        return "";
    }
private:
    std::string def; // default name
    std::map<std::string, Type> types;
};

struct Extend {
    int flag;
    int ctrl_flag;
    const Alias *alias;

    Extend(int _flag, const Alias *_alias):flag(_flag), ctrl_flag(0), alias(_alias) {
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

