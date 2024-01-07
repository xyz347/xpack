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

#ifndef __X_PACK_JSON_DECODER_H
#define __X_PACK_JSON_DECODER_H

#include <fstream>

#include "rapidjson_custom.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "xdecoder.h"
#include "json_data.h"


namespace xpack {


class JsonNode {
    typedef XDecoder<JsonNode> decoder;
public:
    typedef rapidjson::Value::ConstMemberIterator Iterator;

    JsonNode(const rapidjson::Value* val=NULL):v(val){}

    // convert JsonData to JsonNode
    // The life cycle of jd cannot be shorter than JsonNode
    JsonNode(const JsonData&jd):v(jd.current){}

    inline static const char * Name() {
        return "json";
    }
    operator bool() const {
        return v != NULL;
    }
    bool IsNull() const {
        return v!=NULL && v->IsNull();
    }
    JsonNode Find(decoder&de, const char*key, const Extend *ext) const {
        (void)ext;
        if (v->IsNull()) {
            return JsonNode();
        } else if (!v->IsObject()) {
            de.decode_exception("not object", NULL);
        }
        rapidjson::Value::ConstMemberIterator iter = v->FindMember(key);
        if (iter != v->MemberEnd()) {
            return JsonNode(&iter->value);
        } else {
            return JsonNode();
        }
    }
    size_t Size(decoder&de) const {
        if (v->IsNull()) {
            return 0;
        } else if (!v->IsArray()) {
            de.decode_exception("not array", NULL);
        }
        return (size_t)v->Size();
    }
    JsonNode At(size_t index) const { // no exception
        return JsonNode(&(*v)[(rapidjson::SizeType)index]);
    }
    JsonNode Next(decoder&de, const JsonNode&parent, Iterator&iter, std::string&key) const {
        if (!parent.v->IsObject()) {
            de.decode_exception("not object", NULL);
        }

        if (v != parent.v) {
            ++iter;
        } else  {
            iter = parent.v->MemberBegin();
        }

        if (iter != parent.v->MemberEnd()) {
            key = iter->name.GetString();
            return JsonNode(&iter->value);
        }

        return JsonNode();
    }

    bool Get(decoder&de, std::string&val, const Extend*ext) {
        (void)ext;
        if (v->IsString()) {
            val = std::string(v->GetString(), v->GetStringLength());
        } else if (!v->IsNull()) {
            de.decode_exception("not string", NULL);
        }
        return true;
    }
    bool Get(decoder&de, bool &val, const Extend*ext) {
        (void)ext;
        if (v->IsBool()) {
            val = v->GetBool();
        } else if (v->IsInt64()) {
            val = (0 != (v->GetInt64()));
        } else if (v->IsNull()) {
            val = false;
        } else {
            de.decode_exception("not bool or integer", NULL);
        }
        return true;
    }
    template <class T>
    typename x_enable_if<numeric<T>::is_integer, bool>::type Get(decoder&de, T &val, const Extend*ext){
        (void)ext;
        if (v->IsInt()) {
            val = (T)v->GetInt();
        } else if (v->IsUint()) {
            val = (T)v->GetUint();
        } else if (v->IsInt64()) {
            val = (T)v->GetInt64();
        } else if (v->IsUint64()) {
            val = (T)v->GetUint64();
        } else if (v->IsNull()) {
            val = 0;
        } else {
            de.decode_exception("not integer", NULL);
        }
        return true;
    }
    template <class T>
    typename x_enable_if<numeric<T>::is_float, bool>::type Get(decoder&de, T &val, const Extend*ext){
        (void)ext;
        if (v->IsNumber()) {
            val = (T)v->GetDouble();
        } else if (v->IsNull()) {
            val = 0;
        } else {
            de.decode_exception("not number", NULL);
        }
        return true;
    }

    ////////////// for JsonData ///////////////////////
    bool decode_type_spec(JsonData& val, const Extend *ext) {
        (void)ext;
        val.reset(v);
        return true;
    }

private:
    const rapidjson::Value* v;
    rapidjson::Value::ConstMemberIterator iter;
};

class JsonDecoder {
public:
    template <class T>
    bool decode(const std::string&str, T&val) {
        rapidjson::Document doc;
        if (this->parse(str, doc)) {
            JsonNode node(&doc);
            return XDecoder<JsonNode>(NULL, (const char*)NULL, node).decode(val, NULL);
        }
        return false;
    }
    template <class T>
    bool decode_file(const std::string&fname, T&val) {
        std::string data;
        bool ret = Util::readfile(fname, data);
        if (ret) {
            ret =  this->decode(data, val);
        }
        return ret;
    }
private:
    bool parse(const std::string&data, rapidjson::Document &doc) {
        std::string err;

        const unsigned int parseFlags = rapidjson::kParseNanAndInfFlag;
        doc.Parse<parseFlags>(data.data(), data.length());

        if (doc.HasParseError()) {
            size_t offset = doc.GetErrorOffset();
            std::string parse_err(rapidjson::GetParseError_En(doc.GetParseError()));
            std::string err_data = data.substr(offset, 32);
            err = "Parse json fail. err="+parse_err+". offset="+err_data;
            throw std::runtime_error(err);
        }
        return true;
    }
};

// /////////////// JsonData ///////////////////
template<>struct is_xpack_type_spec<JsonNode, JsonData> {static bool const value = true;};

template <typename T>
inline T JsonData::Get() const {
    JsonNode node(*this);
    T t;
    XDecoder<JsonNode>(NULL, (const char*)NULL, node).decode(t, NULL);
    return t;
}
template <typename T>
inline bool JsonData::Get(T&val) const {
    JsonNode node(*this);
    return XDecoder<JsonNode>(NULL, (const char*)NULL, node).decode(val, NULL);
}

}

#endif
