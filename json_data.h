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

// wrapper for rapidjson::Value.
// If we use other json parser someday, users won’t have to modify the code.
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

// Currently JsonData is read-only and may support modification in the future
class JsonData:private noncopyable {
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
        JsonData& Val() const {
            return _parent->member(&(_iter->value), *(_parent->alloc()));
        }
    private:
        rapidjson::Value::ConstMemberIterator _iter;
        JsonData* _parent;
    };
public:
    typedef MemberIterator Iterator;
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

    JsonData& operator[](size_t index) {
        JsonData *d = alloc();
        if (NULL != _val && _val->IsArray()) {
            if (index < (size_t)_val->Size()) {
                d->_val = &(*_val)[(rapidjson::SizeType)index];
            } else {
                // TODO decode_exception("Out of index", NULL);
            }
        } else {
            // TODO decode_exception("not array", NULL);
        }

        return *d;
    }

    JsonData& operator[](const char*key) {
        JsonData *d = alloc();
        if (NULL != _val && _val->IsObject()) {
            rapidjson::Value::ConstMemberIterator iter;
            if (_val->MemberEnd() != (iter=_val->FindMember(key))) {
                d->_val = &(iter->value);
            }
        } else {
            // TODO decode_exception("not object", key);
        }
        return *d;
    }

    // iter
    Iterator Begin() {
        return Iterator(_val->MemberBegin(), this);
    }
    Iterator End() {
        return Iterator(_val->MemberEnd(), this);
    }

    // JsonData is noncopyable, if need to pass it outside the function, use Swap
    // DO NOT Swap child node. JsonData[0].Swap will crash
    // If the compiler supports C++11, you should use shared_ptr<JsonData> instead of Swap
    void Swap(JsonData& d) {
	    rapidjson::MemoryPoolAllocator<> *allocator = _allocator;
    	const rapidjson::Value *val = _val;
    	bool alloc = _alloc;
    	std::string raw_data = _raw_data;

        _allocator = d._allocator;
        _val = d._val;
        _alloc = d._alloc;
        _raw_data = d._raw_data;

        d._allocator = allocator;
        d._val = val;
        d._alloc = alloc;
        d._raw_data = raw_data;
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
    JsonData():_allocator(NULL),_val(NULL), _alloc(false) {
    }
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
            for (rapidjson::Value::ConstMemberIterator iter = _val->MemberBegin(); iter!=_val->MemberEnd(); ++iter){
                JsonData d;
                d._val = &iter->value;
                d.xpack_encode(obj, iter->name.GetString(), ext);
            }
            obj.ObjectEnd(key, ext);
            break;
        case kArrayType:{
                obj.ArrayBegin(key, ext);
                size_t max = Size();
                for (size_t i = 0; i<max; ++i) {
                    JsonData d;
                    d._val = &(*_val)[(rapidjson::SizeType)i];
                    d.xpack_encode(obj, NULL, ext);
                }
                obj.ArrayEnd(key, ext);
            }
            break;
        }
        return true;
    }

private:
    JsonData(const rapidjson::Value *v):_allocator(NULL), _val(v), _alloc(false) {
    }
    JsonData* alloc() {
        JsonData *d = new JsonData();
        _collector.push_back(d);
        return d;
    }
    // after xpack::json::decode, JsonDecoder will destruct, so we need copy data
    // to JsonData, and copy can only be called by decode
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
        for (size_t i=0; i<_collector.size(); ++i) {
            delete _collector[i];
        }
        _collector.clear();
    }
    JsonData& member(const rapidjson::Value *v, JsonData&d) const {
        d._val = v;
        return d;
    }

    rapidjson::MemoryPoolAllocator<> *_allocator;
    const rapidjson::Value *_val;
    bool _alloc;

    std::vector<JsonData*> _collector;

    std::string _raw_data;
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
