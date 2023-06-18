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


#ifndef __X_PACK_BSON_ENCODER_H
#define __X_PACK_BSON_ENCODER_H

#include <libbson-1.0/bson.h>

#include "xpack/util.h"
#include "xpack/xencoder.h"
#include "bson_type.h"

namespace xpack {

class BsonWriter: private noncopyable {
    friend class BsonBuilder;
    struct Node {
        const char *key;
        Node    *parent;
        bson_t data;
        
        Node(const char *_key = NULL, Node *_parent=NULL):key(_key), parent(_parent) {
            if (NULL == parent) {
                bson_init(&data);
            }
        }
        ~Node() {
            if (NULL == parent) {
                bson_destroy(&data);
            }
        }
    };
    friend class XEncoder<BsonWriter>;
    friend class BsonEncoder;
    const static bool support_null = true;
public:
    BsonWriter() {
        cur = new Node(NULL, NULL);
        nodes.push_back(cur);
    }

    ~BsonWriter() {
        std::list<Node*>::iterator it;
        for (it = nodes.begin(); it!=nodes.end(); ++it) {
            delete *it;
        }
    }
private:
    inline static const char *Name() {
        return "bson";
    }
    inline const char *IndexKey(size_t index) {
        if (index > indexStr.size()+1) {
            std::string s = Util::itoa(index);
            errCase.push_back(s);
            return errCase.back().c_str();
        }
        if (index >= indexStr.size()) {
            std::string s = Util::itoa(index);
            indexStr.push_back(s);
        }
        return indexStr[index].c_str();
    }
    // return bson binary data
    std::string String() const {
        return std::string((const char*)bson_get_data(&cur->data), cur->data.len);
    }
    // return json format string
    std::string Json() const {
        size_t len;
        char *jstr = bson_as_json(&cur->data, &len);
        std::string ret(jstr);
        bson_free(jstr);

        return ret;
    }

    void ArrayBegin(const char *key, const Extend *ext) {
        (void)ext;
        if (key != NULL) {
            Node *tmp = new Node(key, cur);
            nodes.push_back(tmp);

            bson_append_array_begin(&cur->data, key, strlen(key), &tmp->data);
            cur = tmp;
        }
    }
    void ArrayEnd(const char *key, const Extend *ext) {
        (void)ext;
        if (NULL != key) {
            bson_append_array_end(&cur->parent->data, &cur->data);
            cur = cur->parent; // do not pop and delete
        }
    }
    void ObjectBegin(const char *key, const Extend *ext) {
        (void)ext;
        if (key != NULL) {
            Node *tmp = new Node(key, cur);
            nodes.push_back(tmp);

            bson_append_document_begin(&cur->data, key, strlen(key), &tmp->data);
            cur = tmp;
        }
    }
    void ObjectEnd(const char *key, const Extend *ext) {
        (void)ext;
        if (NULL != key) { // in case of inherit, object key is NULL
            bson_append_document_end(&cur->parent->data, &cur->data);
            cur = cur->parent; // do not pop and delete
        }
    }
    bool WriteNull(const char*key, const Extend *ext) {
        (void)ext;
        bson_append_null(&cur->data, key, strlen(key));
        return true;
    }
    bool encode_bool(const char*key, const bool &val, const Extend *ext) {
        (void)ext;
        bson_append_bool(&cur->data, key, strlen(key), val);
        return true;
    }
    bool encode_string(const char*key, const char*val, size_t length, const Extend *ext) {
        (void)ext;
        bson_append_utf8(&cur->data, key, strlen(key), val, length);
        return true;
    }
    bool encode_string(const char*key, const std::string& val, const Extend *ext) {
        (void)ext;
        bson_append_utf8(&cur->data, key, strlen(key), (const char*)val.data(), val.length());
        return true;
    }
    bool encode_string(const char*key, const char* val, const Extend *ext) {
        (void)ext;
        if (NULL != val) {
            bson_append_utf8(&cur->data, key, strlen(key), val, strlen(val));
        } else {
            bson_append_utf8(&cur->data, key, strlen(key), "", 0);
        }
        return true;
    }

    #define X_PACK_BSON_ENCODE_NUMBER(vtype, ftype, ctype) \
    inline bool encode_number(const char*key, const vtype&val, const Extend*ext) {\
        (void)ext;\
        bson_append_##ftype(&cur->data, key, strlen(key), (vtype)val);\
        return true; \
    }
    X_PACK_BSON_ENCODE_NUMBER(char, int32, int32_t)
    X_PACK_BSON_ENCODE_NUMBER(signed char, int32, int32_t)
    X_PACK_BSON_ENCODE_NUMBER(unsigned char, int32, int32_t)
    X_PACK_BSON_ENCODE_NUMBER(short, int32, int32_t)
    X_PACK_BSON_ENCODE_NUMBER(unsigned short, int32, int32_t)
    X_PACK_BSON_ENCODE_NUMBER(int, int32, int32_t)
    X_PACK_BSON_ENCODE_NUMBER(unsigned int, int32, int32_t)

    X_PACK_BSON_ENCODE_NUMBER(long, int64, int64_t)
    X_PACK_BSON_ENCODE_NUMBER(unsigned long, int64, int64_t)
    X_PACK_BSON_ENCODE_NUMBER(long long, int64, int64_t)
    X_PACK_BSON_ENCODE_NUMBER(unsigned long long, int64, int64_t)

    X_PACK_BSON_ENCODE_NUMBER(float, double, double)
    X_PACK_BSON_ENCODE_NUMBER(double, double, double)
    X_PACK_BSON_ENCODE_NUMBER(long double, double, double)

    // bson types
    bool encode_type_spec(const char*key, const bson_oid_t &val, const Extend *ext) {
        (void)ext;
        bson_append_oid(&cur->data, key, strlen(key), &val);
        return true;
    }
    bool encode_type_spec(const char*key, const bson_date_time_t &val, const Extend *ext) {
        (void)ext;
        bson_append_date_time(&cur->data, key, strlen(key), val.ts);
        return true;
    }
    bool encode_type_spec(const char *key, const bson_timestamp_t &val, const Extend *ext) {
        (void)ext;
        bson_append_timestamp(&cur->data, key, strlen(key), val.timestamp, val.increment);
        return true;
    }
    bool encode_type_spec(const char *key, const bson_decimal128_t &val, const Extend *ext) {
        (void)ext;
        return bson_append_decimal128(&cur->data, key, strlen(key), &val);
    }
    bool encode_type_spec(const char *key, const bson_regex_t &val, const Extend *ext) {
        (void)ext;
        return bson_append_regex(&cur->data, key, strlen(key), val.pattern.c_str(), val.options.c_str());
    }
    bool encode_type_spec(const char *key, const bson_binary_t &val, const Extend *ext) {
        (void)ext;
        return bson_append_binary(&cur->data, key, strlen(key), val.subType, (const uint8_t*)val.data.data(), val.data.length());
    }

private:
    std::vector<std::string> indexStr;
    std::list<std::string> errCase;
    std::list<Node*> nodes;
    Node *cur;
};

class BsonEncoder {
public:
    BsonEncoder() {
    }

    template <class T>
    std::string encode(const T&val) {
        BsonWriter wr;
        XEncoder<BsonWriter> en(wr);
        en.encode(NULL, val, NULL);
        return wr.String();
    }

    template <class T>
    std::string encode_as_json(const T&val) {
        BsonWriter wr;
        XEncoder<BsonWriter> en(wr);
        en.encode(NULL, val, NULL);
        return wr.Json();
    }
};

template<>struct is_xpack_type_spec<BsonWriter, bson_oid_t> {static bool const value = true;};
template<>struct is_xpack_type_spec<BsonWriter, bson_date_time_t> {static bool const value = true;};
template<>struct is_xpack_type_spec<BsonWriter, bson_timestamp_t> {static bool const value = true;};
template<>struct is_xpack_type_spec<BsonWriter, bson_decimal128_t> {static bool const value = true;};
template<>struct is_xpack_type_spec<BsonWriter, bson_binary_t> {static bool const value = true;};
template<>struct is_xpack_type_spec<BsonWriter, bson_regex_t> {static bool const value = true;};

}

#endif
