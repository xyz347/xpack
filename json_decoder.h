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
#include "thirdparty/rapidjson/document.h"
#include "thirdparty/rapidjson/error/en.h"

#include "xdecoder.h"

namespace xpack {

class JsonDecoder:public XDecoder<JsonDecoder> {
public:
    friend class XDecoder<JsonDecoder>;
    using xdoc_type::decode;

    JsonDecoder(const std::string& str, bool isfile=false):xdoc_type(NULL, ""),_doc(new rapidjson::Document),_val(_doc) {
        std::string err;
        std::string data;

        do {
            if (isfile) {
                std::ifstream fs(str.c_str(), std::ifstream::binary);
                if (!fs) {
                    err = "Open file["+str+"] fail.";
                    break;
                }
                std::string _tmp((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
                data.swap(_tmp);
                _doc->Parse(data);
            } else  {
                _doc->Parse(str);
            }

            if (_doc->HasParseError()) {
                size_t offset = _doc->GetErrorOffset();
                std::string parse_err(rapidjson::GetParseError_En(_doc->GetParseError()));
                if  (isfile) {
                    std::string err_data = data.substr(offset, 32);
                    err = "Parse json file \""+str+"\" fail. err="+parse_err+". offset="+err_data;
                    break;
                } else {
                    std::string err_data = str.substr(offset, 32);
                    err = "Parse json string \""+str+"\" fail. err="+parse_err+". offset="+err_data;
                    break;
                }
            }
            init();
            return;
        } while (false);

        delete _doc;
        _doc = NULL;
        throw std::runtime_error(err);
    }

    ~JsonDecoder() {
        if (NULL != _doc) {
            delete _doc;
            _doc = NULL;
        }
        if (NULL != _iter) {
            delete _iter;
            _iter = NULL;
        }
    }

    inline const char * Type() const {
        return "json";
    }

public:
    // decode
    #define XPACK_JSON_DECODE(f, ...)                           \
        const rapidjson::Value *v = get_val(key);               \
        bool ret = false;                                       \
        if (NULL != v) {                                        \
            try {                                               \
                val = __VA_ARGS__ v->f();                       \
            } catch (const std::exception&e) {                  \
                decode_exception("type unmatch", key);          \
            }                                                   \
            ret = true;                                         \
        } else if (NULL!=key && Extend::Mandatory(ext)) {       \
            decode_exception("mandatory key not found", key);   \
        }                                                       \
        return ret


    bool decode(const char*key, std::string &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetString);
    }
    bool decode(const char*key, bool &val, const Extend *ext) {
        const rapidjson::Value *v = get_val(key);
        if (NULL == v) {
            if (NULL!=key && Extend::Mandatory(ext)) {
                decode_exception("mandatory key not found", key);
            }
            return false;
        } else if (v->IsBool()) {
            val = v->GetBool();
            return true;
        } else if (v->IsInt64()) {
            val = (0 != (v->GetInt64()));
            return true;
        } else {
            decode_exception("wish bool, but not bool or int", key);
            return false;
        }
    }
    bool decode(const char*key, char &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetInt, (char));
    }
    bool decode(const char*key, signed char &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetInt, (char));
    }
    bool decode(const char*key, unsigned char &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetInt, (unsigned char));
    }
    bool decode(const char*key, short &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetInt, (short));
    }
    bool decode(const char*key, unsigned short &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetInt, (unsigned short));
    }
    bool decode(const char*key, int &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetInt);
    }
    bool decode(const char*key, unsigned int &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetUint);
    }
    bool decode(const char*key, long &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetInt64, (long));
    }
    bool decode(const char*key, unsigned long &val, const Extend *ext) {
       XPACK_JSON_DECODE(GetUint64, (unsigned long));
    }
    bool decode(const char*key, long long &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetInt64, (long long));
    }
    bool decode(const char*key, unsigned long long &val, const Extend *ext) {
       XPACK_JSON_DECODE(GetUint64, (unsigned long long));
    }
    bool decode(const char*key, float &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetFloat);
    }
    bool decode(const char*key, double &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetDouble);
    }
    bool decode(const char*key, long double &val, const Extend *ext) {
        XPACK_JSON_DECODE(GetDouble, (long double));
    }

    // array
    size_t Size() {
        if (_val->IsArray()) {
            return (size_t)_val->Size();
        } else {
            return 0;
        }
    }
    JsonDecoder At(size_t index) {
        if (_val->IsArray()) {
            return JsonDecoder(&(*_val)[(rapidjson::SizeType)index], this, index);
        } else {
            throw std::runtime_error("Out of index");
        }
        return JsonDecoder(NULL, NULL, "");
    }

    // iter
    JsonDecoder* Find(const char*key, JsonDecoder*tmp) {
        rapidjson::Value::ConstMemberIterator iter;
        if (NULL!=_val && _val->MemberEnd()!=(iter=_val->FindMember(key)) && !(iter->value.IsNull())) {
            tmp->_key = key;
            tmp->_parent = this;
            tmp->_val = &iter->value;
            return tmp;
        } else {
            return NULL;
        }
    }

    JsonDecoder Begin() {
        if (_iter != NULL) {
            delete _iter;
        }
        _iter = new(rapidjson::Value::ConstMemberIterator);
        *_iter = _val->MemberBegin();
        if (*_iter != _val->MemberEnd()) {
            return JsonDecoder(&(*_iter)->value, this, (*_iter)->name.GetString());
        } else {
            return JsonDecoder(NULL, this, "");
        }
    }
    JsonDecoder Next() {
        if (NULL == _parent) {
            throw std::runtime_error("parent null");
        } else if (NULL == _parent->_iter) {
            throw std::runtime_error("parent no iter");
        } else {
            ++(*_parent->_iter);
        }
        if (*_parent->_iter != _parent->_val->MemberEnd()) {
            return JsonDecoder(&(*_parent->_iter)->value, _parent, (*_parent->_iter)->name.GetString());
        } else {
            return JsonDecoder(NULL, _parent, "");
        }
    }
    operator bool() const {
        return NULL != _val;
    }

private:
    JsonDecoder():xdoc_type(NULL, ""),_doc(NULL),_val(NULL) {
        init();
    }
    JsonDecoder(const rapidjson::Value* val, const JsonDecoder*parent, const char*key):xdoc_type(parent, key),_doc(NULL),_val(val) {
        init();
    }
    JsonDecoder(const rapidjson::Value* val, const JsonDecoder*parent, size_t index):xdoc_type(parent, index),_doc(NULL),_val(val) {
        init();
    }
    void init() {
        _iter = NULL;
    }

    const rapidjson::Value* get_val(const char *key) {
        if (NULL == key) {
            return _val;
        } else if (NULL != _val) {
            rapidjson::Value::ConstMemberIterator iter = _val->FindMember(key);
            if (iter != _val->MemberEnd() && !(iter->value.IsNull())) {
                return &iter->value;
            } else {
                return NULL;
            }
        } else {
            return NULL;
        }
    }

    rapidjson::Document* _doc;
    const rapidjson::Value* _val;
    mutable rapidjson::Value::ConstMemberIterator* _iter;
};

}

#endif
