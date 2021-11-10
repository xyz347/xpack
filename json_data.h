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

// wrapper for rapidjson::Value 
// If we use other json parser someday, users won’t have to modify the code
// Most of the code is copied from rapidjson

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
public:
    // check type
    JsonType Type() const {
        if (_val == NULL) {
            return kNullType;
        }
        return static_cast<JsonType>(_val->GetType());
    }
    bool IsNull()   const { return _val->IsNull(); }
    bool IsBool()   const { return _val->IsBool(); }
    bool IsObject() const { return _val->IsObject(); }
    bool IsArray()  const { return _val->IsArray(); }
    bool IsNumber() const { return _val->IsNumber(); }
    bool IsDouble() const { return _val->IsDouble(); }
    bool IsString() const { return _val->IsString(); }

    std::string GetString() const {return _val->GetString(); }
    bool GetBool() const {return _val->GetBool();}
    int GetInt() const {return _val->GetInt();}
    unsigned int GetUint() const {return _val->GetUint();}
    int64_t GetInt64() const {return _val->GetInt64();}
    uint64_t GetUint64() const {return _val->GetUint64();}
    float GetFloat() const {return _val->GetFloat();}
    double GetDouble() const {return _val->GetDouble();}

    // check is valid JsonData
    operator bool() const {
        return NULL != _val;
    }

    // get array size
    size_t Size() const {
        if (_val->IsArray()) {
            return (size_t)_val->Size();
        } else {
            return 0;
        }
    }
    // get item from array
    JsonData operator[] (size_t index) const {
        return JsonData(&(*_val)[(rapidjson::SizeType)index]);
    }
    // get item from object
    JsonData operator[] (const char* key) const {
        return (*this)[std::string(key)];
    }
    JsonData operator[] (const std::string& key) const {
        rapidjson::Value::ConstMemberIterator iter;
        if (_val->MemberEnd() != (iter=_val->FindMember(key))) {
            return JsonData(&(iter->value));
        } else {
            return JsonData();
        }
    }

    // iter item of object
    JsonData Begin() const {
        rapidjson::Value::ConstMemberIterator iter = _val->MemberBegin();
        if (iter != _val->MemberEnd()) {
            return JsonData(iter, _val);
        } else {
            return JsonData();
        }
    }
    JsonData Next() const {
        rapidjson::Value::ConstMemberIterator iter = _iter+1;
        if (iter != _parent->MemberEnd()) {
            return JsonData(iter, _parent);
        } else {
            return JsonData();
        }
    }
    // only for iter
    std::string Key() const {
        return _iter->name.GetString();
    }

    template <class T>
    bool decode(T &val) const {
        JsonDecoder d(_val);
        return d.decode(NULL, val, NULL);
    }

    std::string String() {
        if (_raw_data.empty()) {
            JsonEncoder e(-1);
            xpack_encode(e, NULL, NULL);
            _raw_data = e.String();
        }
        return _raw_data;
    }
public:
    //friend class JsonDecoder;
    JsonData():_allocator(NULL),_val(NULL), _parent(NULL), _alloc(false) {}
    ~JsonData() {
        reset();
        if (NULL != _allocator) {
            delete _allocator;
            _allocator = NULL;
        }
    }

    bool xpack_decode(JsonDecoder &obj, const char*key, const Extend *ext) {
        const rapidjson::Value *v = obj.get_val(key);
        if (NULL == v) {
            if (NULL!=key && Extend::Mandatory(ext)) {
                obj.decode_exception("mandatory key not found", key);
            }
            return false;
        } else {
            copy(v);
            return true;
        }
    }
    bool xpack_encode(JsonEncoder &obj, const char*key, const Extend *ext) const {
        switch (Type()){
        case kNullType:
            return false; // not support write null now
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
            for (JsonData d = Begin(); d; d=d.Next()){
                d.xpack_encode(obj, d.Key().c_str(), ext);
            }
            obj.ObjectEnd(key, ext);
            break;
        case kArrayType:{
                obj.ArrayBegin(key, ext);
                size_t max = Size();
                for (size_t i = 0; i<max; ++i) {
                    (*this)[i].xpack_encode(obj, NULL, ext);
                }
                obj.ArrayEnd(key, ext);
            }
            break;
        }
        return true;
    }
private:
    JsonData(const rapidjson::Value *v):_allocator(NULL), _val(v), _parent(NULL), _alloc(false) {}
    JsonData(rapidjson::Value::ConstMemberIterator &iter, const rapidjson::Value *p):_allocator(NULL), _parent(p), _alloc(false), _iter(iter) {
        _val = &(_iter->value);
    }
    void copy(const rapidjson::Value *v) {
        if (NULL == _allocator) {
            _allocator = new rapidjson::MemoryPoolAllocator<>();
        }
        reset();
        _val = new rapidjson::Value(*v, *_allocator, true);
        _alloc = true;
    }
    void reset() {
        if (_alloc && _val!=NULL) {
            delete _val;
            _val = NULL;
            _alloc = false;
        }
    }
    rapidjson::MemoryPoolAllocator<> *_allocator;
    const rapidjson::Value *_val;
    const rapidjson::Value *_parent;
    bool _alloc;
    std::string _raw_data;

    rapidjson::Value::ConstMemberIterator _iter;
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
