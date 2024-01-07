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

#ifndef __X_PACK_YAML_ENCODER_H
#define __X_PACK_YAML_ENCODER_H

#include <string>

#include "yaml-cpp/yaml.h"

#include "xencoder.h"

namespace xpack {

class YamlWriter:private noncopyable {
    friend class XEncoder<YamlWriter>;
    friend class YamlEncoder;

    const static bool support_null = true;
public:
    YamlWriter(size_t indent = 2, int maxDecimalPlaces = -1) {
        e.SetIndent(indent);
        if (maxDecimalPlaces > 0) {
            e.SetFloatPrecision((size_t)maxDecimalPlaces);
            e.SetDoublePrecision((size_t)maxDecimalPlaces);
        }
    }
    ~YamlWriter() {
    }
private:
    inline static const char *Name() {
        return "yaml";
    }
    inline const char *IndexKey(size_t index) {
        (void)index;
        return NULL;
    }
    std::string String() {
        return std::string(e.c_str(), e.size());
    }

    void ArrayBegin(const char *key, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        e << YAML::Value << YAML::BeginSeq;
    }
    void ArrayEnd(const char *key, const Extend *ext) {
        (void)key;
        (void)ext;
        e << YAML::EndSeq;
    }
    void ObjectBegin(const char *key, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        e << YAML::Value << YAML::BeginMap;
    }
    void ObjectEnd(const char *key, const Extend *ext) {
        (void)key;
        (void)ext;
        e << YAML::EndMap;
    }
    bool WriteNull(const char*key, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        e << YAML::Value << YAML::Null;
        return true;
    }
    bool encode_bool(const char*key, const bool&val, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        e << YAML::Value << val;
        return true; 
    }
    bool encode_string(const char*key, const std::string&val, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        if (!val.empty()) {
            e << YAML::Value << val;
        } else {
            e << YAML::Value << YAML::Null;
        }
        return true; 
    }
    template <typename T>
    typename x_enable_if<numeric<T>::value, bool>::type encode_number(const char*key, const T&val, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        e << YAML::Value << val;
        return true; 
    }

    void xpack_set_key(const char*key) { // openssl defined set_key macro, so we named it xpack_set_key
        if (NULL!=key && key[0]!='\0') {
            e << YAML::Key << key;
        }
    }

    YAML::Emitter e;
};

class YamlEncoder {
public:
    YamlEncoder() {
        indentCount = 2;
        maxDecimalPlaces = -1;
    }
    
    YamlEncoder& SetIndent(size_t indent) {
        indentCount = indent;
        return *this;
    }
    YamlEncoder& SetMaxDecimalPlaces(int _maxDecimalPlaces) {
        maxDecimalPlaces = _maxDecimalPlaces;
        return *this;
    }

    template <class T>
    std::string encode(const T&val) {
        YamlWriter wr(indentCount, maxDecimalPlaces);
        XEncoder<YamlWriter> en(wr);
        en.encode(NULL, val, NULL);
        return wr.String();
    }

private:
    size_t indentCount;
    int maxDecimalPlaces;
};


}

#endif
