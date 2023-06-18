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


#include "rapidjson_custom.h"
#include "rapidjson/document.h"
#include "util.h"
#include "traits.h"

namespace xpack {

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

class JsonData {
    friend class JsonNode;
    friend class JsonWriter;
public:
    JsonData():current(NULL){}

    // check is valid JsonData
    operator bool() const {
        return NULL != current;
    }
    JsonType Type() const {
        if (NULL != current) {
            return static_cast<JsonType>(current->GetType());
        }
        return kNullType; // ?
    }
    bool IsNull()   const { return NULL==current||current->IsNull(); }
    bool IsBool()   const { return NULL!=current && current->IsBool(); }
    bool IsObject() const { return NULL!=current && current->IsObject(); }
    bool IsArray()  const { return NULL!=current && current->IsArray(); }
    bool IsNumber() const { return NULL!=current && current->IsNumber(); }
    bool IsDouble() const { return NULL!=current && current->IsDouble(); }
    bool IsString() const { return NULL!=current && current->IsString(); }
    size_t Size() const {return (size_t)current->Size();}

    JsonData operator [](const char *key) const {
        if (NULL != current && current->IsObject()) {
            rapidjson::Value::ConstMemberIterator iter = current->FindMember(key);
            if (iter != current->MemberEnd()) {
                return JsonData(&iter->value);
            } // no exception ?
        }
        return JsonData();
    }
    JsonData operator [](size_t index) const {
        if (NULL != current && current->IsArray() && index < (size_t)current->Size()) {
            return JsonData(&(*current)[(rapidjson::SizeType)index]);
        }
        return JsonData();
    }

    std::string String() const; // Implementation in json_encoder.h

    template <typename T>       // For base byte(integer/float/double/bool/std::string), implementation in json_decoder.h
    T Get() const;

    template <typename T>       // struct, implementation in json_decoder.h
    bool Get(T &val) const;

private:
    JsonData(const rapidjson::Value*v):current(v){}

    void reset(const rapidjson::Value *v) {
        node.reset();
        allocator.reset();
        current = NULL;
        if (NULL != v) {
            if (v->IsNull()) {
                node.reset(new rapidjson::Value());
            } else {
                allocator.reset(new rapidjson::MemoryPoolAllocator<>());
                node.reset(new rapidjson::Value(*v, *allocator, true));
            }
            current = node.get();
        }
    }
    x_shared_ptr<rapidjson::MemoryPoolAllocator<> > allocator;
    x_shared_ptr<rapidjson::Value> node; // release node first

    const rapidjson::Value* current;
};

}

#endif
