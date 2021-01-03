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

#ifndef __X_PACK_JSON_ENCODER_H
#define __X_PACK_JSON_ENCODER_H

#include <string>

#include "rapidjson_custom.h"
#include "thirdparty/rapidjson/prettywriter.h"
#include "thirdparty/rapidjson/stringbuffer.h"

#include "xencoder.h"

namespace xpack {

class JsonEncoder:public XEncoder<JsonEncoder> {
    typedef rapidjson::StringBuffer JSON_WRITER_BUFFER;
    typedef rapidjson::Writer<rapidjson::StringBuffer> JSON_WRITER_WRITER;
    typedef rapidjson::PrettyWriter<rapidjson::StringBuffer> JSON_WRITER_PRETTY;
public:
    friend class XEncoder<JsonEncoder>;
    using xdoc_type::encode;

    JsonEncoder(int indentCount=-1, char indentChar=' ') {
        _buf = new JSON_WRITER_BUFFER;
        if (indentCount < 0) {
            _writer = new JSON_WRITER_WRITER(*_buf);
            _pretty = NULL;
        } else {
            _pretty = new JSON_WRITER_PRETTY(*_buf);
            _pretty->SetIndent(indentChar, indentCount);
            _writer = NULL;
        }
    }
    ~JsonEncoder() {
        if (NULL != _buf) {
            delete _buf;
            _buf = NULL;
        }
        if (NULL != _writer) {
            delete _writer;
            _writer = NULL;
        }
        if (NULL != _pretty) {
            delete _pretty;
            _pretty = NULL;
        }
    }

    inline const char *Type() const {
        return "json";
    }
    inline const char *IndexKey(size_t index) {
        (void)index;
        return NULL;
    }

    std::string String() {
        return _buf->GetString();
    }

public:
    void ArrayBegin(const char *key) {
        xpack_set_key(key);
        if (NULL != _writer) {
            _writer->StartArray();
        } else {
            _pretty->StartArray();
        }
    }
    void ArrayEnd(const char *key) {
        if (NULL != _writer) {
            _writer->EndArray();
        } else {
            _pretty->EndArray();
        }
    }
    void ObjectBegin(const char *key) {
        xpack_set_key(key);
        if (NULL != _writer) {
            _writer->StartObject();
        } else {
            _pretty->StartObject();
        }
    }
    void ObjectEnd(const char *key) {
        if (NULL != _writer) {
            _writer->EndObject();
        } else {
            _pretty->EndObject();
        }
    }

public:
    #define X_PACK_JSON_ENCODE(cond, f)  \
        if ((cond) && Extend::OmitEmpty(ext)){ \
            return false;                \
        }                                \
        xpack_set_key(key);              \
        if (NULL != _writer) {           \
            _writer->f(val);             \
        } else {                         \
            _pretty->f(val);             \
        }                                \
        return true

    bool encode(const char*key, const std::string &val, const Extend *ext) {
        X_PACK_JSON_ENCODE(val.empty(), String);
    }
    bool encode(const char*key, const bool &val, const Extend *ext) {
        X_PACK_JSON_ENCODE(!val, Bool);
    }
    bool encode(const char*key, const char &val, const Extend *ext) {
        return this->encode(key, (const int&)val, ext);
    }
    bool encode(const char*key, const signed char &val, const Extend *ext) {
        return this->encode(key, (const int&)val, ext);
    }
    bool encode(const char*key, const unsigned char &val, const Extend *ext) {
        return this->encode(key, (const unsigned int&)val, ext);
    }
    bool encode(const char*key, const short & val, const Extend *ext) {
        return this->encode(key, (const int&)val, ext);
    }
    bool encode(const char*key, const unsigned short & val, const Extend *ext) {
        return this->encode(key, (const unsigned int&)val, ext);
    }
    bool encode(const char*key, const int& val, const Extend *ext) {
        X_PACK_JSON_ENCODE(val==0, Int);
    }
    bool encode(const char*key, const unsigned int& val, const Extend *ext) {
        X_PACK_JSON_ENCODE(val==0, Uint);
    }
    bool encode(const char*key, const long & val, const Extend *ext) {
        X_PACK_JSON_ENCODE(val==0, Int64);
    }
    bool encode(const char*key, const unsigned long & val, const Extend *ext) {
        X_PACK_JSON_ENCODE(val==0, Uint64);
    }
    bool encode(const char*key, const long long &val, const Extend *ext) {
        return this->encode(key, (const long&)val, ext);
    }
    bool encode(const char*key, const unsigned long long &val, const Extend *ext) {
        return this->encode(key, (const unsigned long&)val, ext);
    }
    bool encode(const char*key, const float & val, const Extend *ext) {
        X_PACK_JSON_ENCODE(val==0, Double);
    }
    bool encode(const char*key, const double & val, const Extend *ext) {
        X_PACK_JSON_ENCODE(val==0, Double);
    }
    bool encode(const char*key, const long double & val, const Extend *ext) {
        X_PACK_JSON_ENCODE(val==0, Double);
    }
private:
    void xpack_set_key(const char*key) { // openssl defined set_key macro, so we named it xpack_set_key
        if (NULL!=key && key[0]!='\0') {
            if (NULL != _writer) {
                _writer->Key(key);
            } else {
                _pretty->Key(key);
            }
        }
    }

    JSON_WRITER_BUFFER* _buf;
    JSON_WRITER_WRITER* _writer;
    JSON_WRITER_PRETTY* _pretty;
};

}

#endif
