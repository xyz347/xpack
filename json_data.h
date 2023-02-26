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

#ifndef __X_PACK_JSON_DATA_H
#define __X_PACK_JSON_DATA_H

/*
It is more reasonable to rely on JsonData for encoder and decoder,
but in order to implement decode and String in JsonData for easy use, 
so the other way around
*/
#include "json_encoder.h"
#include "json_decoder.h"

namespace xpack {

#if defined(X_PACK_SUPPORT_CXX0X)
    template <class T>
    using x_shared_ptr = std::shared_ptr<T>;
#elif defined(_GNU_SOURCE)
// A very crude implementation of shared_ptr
template <class T>
class x_shared_ptr {
public:
    x_shared_ptr(T *p = NULL):ptr(p) {
        if (NULL != p) {
            counter = new int;
            *counter = 1;
        } else {
            counter = NULL;
        }
    }
    x_shared_ptr(const x_shared_ptr &src) {
        ptr = src.ptr;
        counter = src.counter;
        add_ref(1);
    }
    ~x_shared_ptr() {
        add_ref(-1);
    }
    x_shared_ptr& operator = (const x_shared_ptr &src) {
        add_ref(-1);
        ptr = src.ptr;
        counter = src.counter;
        add_ref(1);
        return *this;
    }
    void reset(T *p = NULL) {
        *this = x_shared_ptr(p);
    }
    T* operator ->() {
        return ptr;
    }
    T* get() {
        return ptr;
    }
    T& operator *() {
        return *ptr;
    }
private:
    void add_ref(int cnt) {
        if (counter == NULL) {
            return;
        }
        int ncnt = __sync_add_and_fetch(counter, cnt);
        if (0 == ncnt) {
            delete counter;
            delete ptr;
        }
    }
    T   *ptr;
    mutable int *counter;
};
#else
    #error need support c++11 or use gnu compiler
#endif

// wrapper for rapidjson::Value.
// If we use other json parser someday, users won't have to modify the code.
// Most of the code is copied from rapidjson.

enum JsonType {
    kNullType = 0,      //!< null
    kFalseType = 1,     //!< false
    kTrueType = 2,      //!< true
    kObjectType = 3,    //!< object
    kArrayType = 4,     //!< array 
    kStringType = 5,    //!< string
    kNumberType = 6     //!< number
};

class JsonEncoder;


class JsonData {
    class MemberIterator {
    public:
        MemberIterator(rapidjson::Value::ConstMemberIterator iter, JsonData* parent):_iter(iter),_parent(parent){}
        bool operator != (const MemberIterator &that) const {
            return _iter != that._iter;
        }
        MemberIterator& operator ++ () {
            ++_iter;
            return *this;
        }
        const char *Key() const {
            return _iter->name.GetString();
        }
        JsonData Val() const {
            JsonData d(*_parent);
            d._res.node = &(_iter->value);
            return d;
        }
    private:
        rapidjson::Value::ConstMemberIterator _iter;
        JsonData* _parent;
    };

    // maintain the memory
    struct Resource {
        x_shared_ptr<rapidjson::MemoryPoolAllocator<> > allocator;
        x_shared_ptr<rapidjson::Value> root; // release root first
        const rapidjson::Value *node; // current node

        Resource() :node(NULL) {
        }
        ~Resource() {
        }

        void reset(const rapidjson::Value *v = NULL, bool initNull = false) {
            root.reset();
            allocator.reset();

            if (NULL != v || initNull) {
                allocator.reset(new rapidjson::MemoryPoolAllocator<>());
                if (NULL != v) {
                    root.reset(new rapidjson::Value(*v, *allocator, true));
                } else {
                    root.reset(new rapidjson::Value());
                }
                node = root.get();
            }
        }
    };
public:
    typedef MemberIterator Iterator;

    // check type
    JsonType Type() const {
        return static_cast<JsonType>(_res.node->GetType());
    }
    bool IsNull()   const { return _res.node->IsNull(); }
    bool IsBool()   const { return _res.node->IsBool(); }
    bool IsObject() const { return _res.node->IsObject(); }
    bool IsArray()  const { return _res.node->IsArray(); }
    bool IsNumber() const { return _res.node->IsNumber(); }
    bool IsDouble() const { return _res.node->IsDouble(); }
    bool IsString() const { return _res.node->IsString(); }

    std::string GetString() const {return _res.node->GetString(); }
    bool GetBool() const {return _res.node->GetBool();}
    int GetInt() const {return _res.node->GetInt();}
    unsigned int GetUint() const {return _res.node->GetUint();}
    int64_t GetInt64() const {return _res.node->GetInt64();}
    uint64_t GetUint64() const {return _res.node->GetUint64();}
    float GetFloat() const {return _res.node->GetFloat();}
    double GetDouble() const {return _res.node->GetDouble();}

    // check is valid JsonData
    operator bool() const {
        return NULL != _res.node;
    }

    // get array size
    size_t Size() const {
        if (_res.node->IsArray()) {
            return (size_t)_res.node->Size();
        } else {
            return 0;
        }
    }

    JsonData operator[](const size_t index) const {
        JsonData d(*this);
        if (NULL != _res.node && _res.node->IsArray()) {
            if (index < (size_t)_res.node->Size()) {
                d._res.node = &(*_res.node)[(rapidjson::SizeType)index];
            } else {
                // TODO decode_exception("Out of index", NULL);
            }
        } else {
            // TODO decode_exception("not array", NULL);
        }

        return d;
    }

    JsonData operator[](const char*key) const {
        JsonData d(*this);
        if (NULL != _res.node && _res.node->IsObject()) {
            rapidjson::Value::ConstMemberIterator iter;
            if (_res.node->MemberEnd() != (iter=_res.node->FindMember(key))) {
                d._res.node = &(iter->value);
            }
        } else {
            // TODO decode_exception("not object", key);
        }
        return d;
    }

    // iter
    Iterator Begin() {
        return Iterator(_res.node->MemberBegin(), this);
    }
    Iterator End() {
        return Iterator(_res.node->MemberEnd(), this);
    }

    template <class T>
    bool decode(T &val) const {
        JsonDecoder d(_res.node);
        return d.decode(NULL, val, NULL);
    }

    std::string String() {
        if (_json_string.empty()) {
            JsonEncoder e(-1);
            xpack_encode(e, NULL, NULL);
            _json_string = e.String();
        }
        return _json_string;
    }
public:
    //friend class JsonDecoder;
    bool xpack_decode(JsonDecoder &obj, const char*key, const Extend *ext) {
        bool isNull;
        const rapidjson::Value *v = obj.get_val(key, isNull);
        if (isNull) {
            _res.reset(NULL, true);
        } else if (NULL == v) {
            if (NULL!=key && Extend::Mandatory(ext)) {
                obj.decode_exception("mandatory key not found", key);
            }
            return false;
        } else {
            _res.reset(v, false);
        }
        return true;
    }
    bool xpack_encode(JsonEncoder &obj, const char*key, const Extend *ext) const {
        switch (Type()){
        case kNullType:
            return obj.writeNull(key, ext);
        case kFalseType:
        case kTrueType:
            return obj.encode(key, GetBool(), ext);
        case kStringType:
            return obj.encode(key, GetString(), ext);
        case kNumberType:
            if (IsDouble()) {
                return obj.encode(key, GetDouble(), ext);
            } else {
                return obj.encode(key, GetInt64(), ext);
            }
        case kObjectType:
            obj.ObjectBegin(key, ext);
            for (rapidjson::Value::ConstMemberIterator iter = _res.node->MemberBegin(); iter!=_res.node->MemberEnd(); ++iter){
                JsonData d;
                d._res.node = &iter->value;
                d.xpack_encode(obj, iter->name.GetString(), ext);
            }
            obj.ObjectEnd(key, ext);
            break;
        case kArrayType:{
                obj.ArrayBegin(key, ext);
                size_t max = Size();
                for (size_t i = 0; i<max; ++i) {
                    JsonData d;
                    d._res.node = &(*_res.node)[(rapidjson::SizeType)i];
                    d.xpack_encode(obj, NULL, ext);
                }
                obj.ArrayEnd(key, ext);
            }
            break;
        }
        return true;
    }

private:
    Resource _res;

    std::string _json_string;
};

template<>
struct is_xpack_xtype<JsonData> {static bool const value = true;};

inline bool xpack_xtype_decode(JsonDecoder &obj, const char*key, JsonData &val, const Extend *ext) {
    return val.xpack_decode(obj, key, ext);
}
inline bool xpack_xtype_encode(JsonEncoder &obj, const char*key, const JsonData &val, const Extend *ext) {
    return val.xpack_encode(obj, key, ext);
}

}

#endif
