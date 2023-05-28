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

#ifndef __X_PACK_BSON_DECODER_H
#define __X_PACK_BSON_DECODER_H

#include <string.h>
#include <libbson-1.0/bson.h>

#include "util.h"
#include "xdecoder.h"
#include "bson_type.h"

namespace xpack {


class BsonNode {
    typedef XDecoder<BsonNode> decoder;
    friend class BsonDecoder;
public:
    typedef size_t Iterator;

    BsonNode(const bson_iter_t* iter = NULL):it(iter),inited(false) {
        if (NULL != it) {
            type = bson_iter_type(it);
        }
    }

    inline static const char * Name() {
        return "bson";
    }
    operator bool () const {
        return it != NULL;
    }
    bool IsNull() const {
        return NULL!=it && BSON_TYPE_NULL==bson_iter_type(it);
    }
    BsonNode Find(decoder&de, const char*key, const Extend *ext) {
        (void)de;
        (void)ext;

        if (!inited) {
            this->init();
        }

        if (BSON_TYPE_DOCUMENT != type) {
            de.decode_exception("not document", NULL);
        }

        node_index::iterator iter = _childs_index.find(key);
        if (iter != _childs_index.end()) {
            return BsonNode(&_childs[iter->second]);
        } else {
            return BsonNode();
        }
    }
    size_t Size(decoder&de) {
        (void)de;
        if (!inited) {
            this->init();
        }

        if (BSON_TYPE_ARRAY == type) {
            return _childs.size();
        } else {
            de.decode_exception("not array", NULL);
            return 0;
        }
    }
    BsonNode At(size_t index) const { // no exception
        return BsonNode(&(_childs[index]));
    }
    BsonNode Next(decoder&de, BsonNode&p, Iterator&iter, std::string&key) {
        (void)de;
        if (!p.inited) {
            p.init();
        }
        if (BSON_TYPE_DOCUMENT != p.type) {
            de.decode_exception("not document", NULL);
        }

        if (it != p.it) {
            ++iter;
        } else {
            iter = 0;
        }

        if (iter != p._childs.size()) {
            bson_iter_t *t = &p._childs[iter];
            key = bson_iter_key(t);
            return BsonNode(t);
        }

        return BsonNode();
    }

    bool Get(decoder&de, std::string&val, const Extend*ext) {
        (void)ext;

        if (BSON_TYPE_UTF8 == type) {
            uint32_t length;
            const char* data = bson_iter_utf8(it, &length);
            if (NULL != data) {
                val = std::string(data, length);
            }
        } else if (BSON_TYPE_NULL != type) {
            de.decode_exception("not string", NULL);
        }
        return true;
    }
    bool Get(decoder&de, bool &val, const Extend*ext) {
        (void)de;
        (void)ext;
        val = bson_iter_as_bool(it);
        return true;
    }
    template <class T>
    typename x_enable_if<numeric<T>::is_integer, bool>::type Get(decoder&de, T &val, const Extend*ext){
        (void)de;
        (void)ext;
        val = (T)bson_iter_as_int64(it);
        return true;
    }
    template <class T>
    typename x_enable_if<numeric<T>::is_float, bool>::type Get(decoder&de, T &val, const Extend*ext){
        (void)de;
        (void)ext;
        val = (T)bson_iter_double(it);
        return true;
    }

    /*std::string json() const {
        if (NULL == _data) {
            return "";
        }
        size_t length=BSON_UINT32_TO_LE(*(int32_t*)_data);
        bson_t b; // local is ok
        bson_init_static(&b, _data, length);

        size_t s;
        char *jstr = bson_as_json(&b, &s);
        if (jstr) {
            std::string j(jstr);
            bson_free(jstr);
            return j;
        } else {
            return "";
        }
    }*/

    // bson type
    bool decode_type_spec(bson_oid_t &val, const Extend *ext) {
        (void)ext;
        const bson_oid_t *t = bson_iter_oid(it);
        if (t != NULL) {
            bson_oid_init_from_data(&val, t->bytes);
            return true;
        } else {
            return false;
        }
    }
    bool decode_type_spec(bson_date_time_t &val, const Extend *ext) {
        (void)ext;
        val.ts = bson_iter_date_time(it);
        return true;
    }
    bool decode_type_spec(bson_timestamp_t &val, const Extend *ext) {
        (void)ext;
        bson_iter_timestamp(it, &val.timestamp, &val.increment);
        return true;
    }
    bool decode_type_spec(bson_decimal128_t &val, const Extend *ext) {
        (void)ext;
        return bson_iter_decimal128(it, &val);
    }
    bool decode_type_spec(bson_regex_t &val, const Extend *ext) {
        (void)ext;
        const char *options = NULL;
        const char *regex = bson_iter_regex(it, &options);
        if (NULL != regex) {
            val.pattern = regex;
        }
        if (NULL != options) {
            val.options = options;
        }
        return true;
    }
    bool decode_type_spec(bson_binary_t &val, const Extend *ext) {
        (void)ext;
        uint32_t len = 0;
        const uint8_t *data = NULL;
        bson_iter_binary(it, &val.subType, &len, &data);
        if (data != NULL && len > 0) {
            val.data = std::string((const char*)data, size_t(len));
        }
        return true;
    }


private:
    typedef std::map<const char*, size_t, cmp_str> node_index; // index of _childs

    // get object or array info
    void init(bool top = false) {
        inited = true;

        bson_iter_t sub;
        if (NULL != it) {
            if (top) {
                type = BSON_TYPE_DOCUMENT;
                memcpy((void*)&sub, (void*)it, sizeof(sub));
            } else if (!bson_iter_recurse(it, &sub)) {
                return;
            }

            size_t i = 0;
            while (bson_iter_next(&sub)) {
                _childs.push_back(sub);
                if (type == BSON_TYPE_DOCUMENT) {
                    _childs_index[bson_iter_key(&sub)] = i++;
                }
            }
        }
    }

    const bson_iter_t* it;
    bson_type_t type;
    bool inited;
    std::vector<bson_iter_t> _childs;  // childs
    node_index _childs_index;
};


class BsonDecoder {
public:
    // if length==0, length will get from data
    template <class T>
    bool decode(const uint8_t*data, size_t length, T&val) {
        length = (length>0)?length:BSON_UINT32_TO_LE(*(int32_t*)data);

        bson_t b;
        bson_iter_t it;
        bson_init_static(&b, data, length);
        bson_iter_init(&it, &b);

        BsonNode node(&it);
        node.init(true);
        return XDecoder<BsonNode>(NULL, (const char*)NULL, node).decode(val, NULL);
    }

    template <class T>
    bool decode(const std::string&data, T&val) {
        return decode((const uint8_t*)data.data(), data.length(), val);
    }
};

template<>struct is_xpack_type_spec<BsonNode, bson_oid_t> {static bool const value = true;};
template<>struct is_xpack_type_spec<BsonNode, bson_date_time_t> {static bool const value = true;};
template<>struct is_xpack_type_spec<BsonNode, bson_timestamp_t> {static bool const value = true;};
template<>struct is_xpack_type_spec<BsonNode, bson_decimal128_t> {static bool const value = true;};
template<>struct is_xpack_type_spec<BsonNode, bson_binary_t> {static bool const value = true;};
template<>struct is_xpack_type_spec<BsonNode, bson_regex_t> {static bool const value = true;};

}

#endif
