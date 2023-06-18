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

#ifndef __X_PACK_YAML_DECODER_H
#define __X_PACK_YAML_DECODER_H

#include "yaml-cpp/yaml.h"
#include "xdecoder.h"

namespace xpack {


class YamlNode {
    typedef XDecoder<YamlNode> decoder;
public:
    typedef YAML::const_iterator Iterator;

    YamlNode(const YAML::Node& node):n(node),valid(true){}
    YamlNode():valid(false){}

    inline static const char * Name() {
        return "yaml";
    }
    operator bool() const {
        return valid;
    }
    bool IsNull() const {
        return valid && n.IsNull();
    }
    YamlNode Find(decoder&de, const char*key, const Extend *ext) const {
        (void)ext;
        if (n.IsNull()) {
            return YamlNode();
        } else if (!n.IsMap()) {
            de.decode_exception("not map", NULL);
        }
        return YamlNode(n[key]);
    }
    size_t Size(decoder&de) const {
        if (!n.IsSequence()) {
            de.decode_exception("not sequence", NULL);
        }
        return (size_t)n.size();
    }
    YamlNode At(size_t index) const { // no exception
        return YamlNode(n[index]);
    }
    YamlNode Next(decoder&de, const YamlNode&parent, Iterator&iter, std::string&key) const {
        if (parent.n.IsMap()) {
            if ((void*)this != (void*)(&parent)) {
                ++iter;
            } else {
                iter = parent.n.begin();
            }
            if (iter != parent.n.end()) {
                key = iter->first.as<std::string>();
                return YamlNode(iter->second);
            }
        } else {
            de.decode_exception("not map", NULL);
        }
        return YamlNode();
    }

    template <class T>
    bool Get(decoder&de, T &val, const Extend*ext){
        (void)de;
        (void)ext;
        try {
            val = n.as<T>();
            return true;
        } catch (const std::exception&e) {
            de.decode_exception(e.what(), NULL);
        } catch (...) {
            de.decode_exception("type unmatch", NULL);
        }
        return false;
    }

private:
    YAML::Node n;
    bool valid;
};

class YamlDecoder {
public:
    template <class T>
    bool decode(const std::string&str, T&val) {
        YAML::Node n = YAML::Load(str);
        if (n) {
            YamlNode node(n);
            return XDecoder<YamlNode>(NULL, (const char*)NULL, node).decode(val, NULL);
        }
        return false;
    }
    template <class T>
    bool decode_file(const std::string&fname, T&val) {
        YAML::Node n = YAML::LoadFile(fname);
        if (n) {
            YamlNode node(n);
            return XDecoder<YamlNode>(NULL, (const char*)NULL, node).decode(val, NULL);
        }
        return false;
    }
};


}

#endif
