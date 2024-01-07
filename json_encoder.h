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
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "xencoder.h"
#include "json_data.h"

namespace xpack {

class JsonWriter:private noncopyable {
    typedef rapidjson::StringBuffer JSON_WRITER_BUFFER;
    typedef rapidjson::Writer<rapidjson::StringBuffer> JSON_WRITER_WRITER;
    typedef rapidjson::PrettyWriter<rapidjson::StringBuffer> JSON_WRITER_PRETTY;

    friend class XEncoder<JsonWriter>;
    friend class JsonEncoder;

    const static bool support_null = true;
public:
    JsonWriter(int indentCount = -1, char indentChar = ' ', int maxDecimalPlaces = -1) {
        _buf = new JSON_WRITER_BUFFER;
        if (indentCount < 0) {
            _writer = new JSON_WRITER_WRITER(*_buf);
            if (maxDecimalPlaces > 0) {
                _writer->SetMaxDecimalPlaces(maxDecimalPlaces);
            }
            _pretty = NULL;
        } else {
            _pretty = new JSON_WRITER_PRETTY(*_buf);
            _pretty->SetIndent(indentChar, indentCount);
            if (maxDecimalPlaces > 0) {
                _pretty->SetMaxDecimalPlaces(maxDecimalPlaces);
            }
            _writer = NULL;
        }
    }
    ~JsonWriter() {
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

private:
    inline static const char *Name() {
        return "json";
    }
    inline const char *IndexKey(size_t index) {
        (void)index;
        return NULL;
    }
    std::string String() {
        return _buf->GetString();
    }

    void ArrayBegin(const char *key, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        if (NULL != _writer) {
            _writer->StartArray();
        } else {
            if (Extend::Flag(ext) & X_PACK_FLAG_SL) {
                _pretty->SetFormatOptions(rapidjson::kFormatSingleLineArray);
            }
            _pretty->StartArray();
        }
    }
    void ArrayEnd(const char *key, const Extend *ext) {
        (void)key;
        (void)ext;
        if (NULL != _writer) {
            _writer->EndArray();
        } else {
            _pretty->EndArray();
            if (Extend::Flag(ext) & X_PACK_FLAG_SL) {
                _pretty->SetFormatOptions(rapidjson::kFormatDefault);
            }
        }
    }
    void ObjectBegin(const char *key, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        if (NULL != _writer) {
            _writer->StartObject();
        } else {
            _pretty->StartObject();
        }
    }
    void ObjectEnd(const char *key, const Extend *ext) {
        (void)key;
        (void)ext;
        if (NULL != _writer) {
            _writer->EndObject();
        } else {
            _pretty->EndObject();
        }
    }
    bool WriteNull(const char*key, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        if (NULL != _writer) {
            _writer->Null();
        } else {
            _pretty->Null();
        }
        return true;
    }
    bool encode_bool(const char*key, const bool&val, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        if (NULL != _writer) {
            _writer->Bool(val);
        } else {
            _pretty->Bool(val);
        }   
        return true; 
    }
    bool encode_string(const char*key, const char*val, size_t length, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        if (NULL != _writer) {
            _writer->String(val, length);
        } else {
            _pretty->String(val, length);
        }   
        return true; 
    }
    bool encode_string(const char*key, const std::string&val, const Extend *ext) {
        return this->encode_string(key, val.data(), val.length(), ext); 
    }
    template <typename T>
    typename x_enable_if<numeric<T>::is_integer && numeric<T>::is_signed, bool>::type encode_number(const char*key, const T&val, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        if (NULL != _writer) {
            _writer->Int64((int64_t)val);
        } else {
            _pretty->Int64((int64_t)val);
        }   
        return true; 
    }
    template <typename T>
    typename x_enable_if<numeric<T>::is_integer && !numeric<T>::is_signed, bool>::type encode_number(const char*key, const T&val, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        if (NULL != _writer) {
            _writer->Uint64((uint64_t)val);
        } else {
            _pretty->Uint64((uint64_t)val);
        }   
        return true; 
    }
    template <typename T>
    typename x_enable_if<numeric<T>::is_float, bool>::type encode_number(const char*key, const T&val, const Extend *ext) {
        (void)ext;
        xpack_set_key(key);
        if (NULL != _writer) {
            _writer->Double((double)val);
        } else {
            _pretty->Double((double)val);
        }   
        return true; 
    }

    // JsonData
    bool encode_type_spec(const char*key, const JsonData&val, const Extend *ext) {
        if (val.current == NULL) {
            return false;
        }
        return this->encode_json_value(key, *val.current, ext);
    }

    bool encode_json_value(const char*key, const rapidjson::Value& val, const Extend *ext) {
        switch (val.GetType()){
        case rapidjson::kNullType:
            return this->WriteNull(key, ext);
        case rapidjson::kFalseType:
        case rapidjson::kTrueType:
            return this->encode_bool(key, val.GetBool(), ext);
        case rapidjson::kStringType:
            return this->encode_string(key, val.GetString(), (size_t)val.GetStringLength(), ext);
        case rapidjson::kNumberType:
            if (val.IsDouble()) {
                return this->encode_number(key, val.GetDouble(), ext);
            } else {
                return this->encode_number(key, val.GetInt64(), ext);
            }
        case rapidjson::kObjectType:
            this->ObjectBegin(key, ext);
            for (rapidjson::Value::ConstMemberIterator iter = val.MemberBegin(); iter!=val.MemberEnd(); ++iter) {
                this->encode_json_value(iter->name.GetString(), iter->value, ext);
            }
            this->ObjectEnd(key, ext);
            break;
        case rapidjson::kArrayType:{
                this->ArrayBegin(key, ext);
                size_t max = (size_t)val.Size();
                for (size_t i = 0; i<max; ++i) {
                    this->encode_json_value(NULL, val[(rapidjson::SizeType)i], ext);
                }
                this->ArrayEnd(key, ext);
            }
            break;
        }
        return true;
    }

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

class JsonEncoder {
public:
    JsonEncoder() {
        indentCount = -1;
        indentChar = ' ';
        maxDecimalPlaces = -1;
    }
    JsonEncoder(int _indentCount, char _indentChar, int _maxDecimalPlaces = -1) { // compat
        indentCount = _indentCount;
        indentChar = _indentChar;
        maxDecimalPlaces = _maxDecimalPlaces;
    }

    void SetMaxDecimalPlaces(int _maxDecimalPlaces) {
        maxDecimalPlaces = _maxDecimalPlaces;
    }

    template <class T>
    std::string encode(const T&val) {
        JsonWriter wr(indentCount, indentChar, maxDecimalPlaces);
        XEncoder<JsonWriter> en(wr);
        en.encode(NULL, val, NULL);
        return wr.String();
    }

private:
    int indentCount;
    char indentChar;
    int maxDecimalPlaces;
};

// //////////////// JsonData  ///////////////////////
template<>struct is_xpack_type_spec<JsonWriter, JsonData> {static bool const value = true;};

inline std::string JsonData::String() const {
    JsonEncoder en;
    return en.encode(*this);
}


}

#endif
