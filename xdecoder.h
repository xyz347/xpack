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


#ifndef __X_PACK_DECODER_H
#define __X_PACK_DECODER_H

#include <stdexcept>
#include <map>
#include <set>
#include <vector>
#include <list>

#include "extend.h"
#include "traits.h"
#include "xtype.h"

#include "string.h"

#ifdef XPACK_SUPPORT_QT
#include <QString>
#include <QList>
#include <QMap>
#include <QVector>
#endif

#ifdef X_PACK_SUPPORT_CXX0X
#include <memory>
#include <unordered_map>
#endif

namespace xpack {


/*
  DOC need implement:
    const char *Type() const; "json/bson/...."

    Iterator;

    Begin(); return iter;
    End();  iter next;
    operator bool;

    size_t Size() const; if doc is array, return array size;
*/
template<class DOC>
class XDecoder {
protected:
    typedef DOC doc_type;
    typedef XDecoder<DOC> xdoc_type;

    struct KeyConverter{
        static inline bool toString(const char *key, std::string &k) {
            k = key;
            return true;
        }
        #ifdef XPACK_SUPPORT_QT
        static inline bool toQString(const char*key, QString &k) {
            k = key;
            return true;
        }
        #endif
    };

public:
    // only c++0x support reference initialize, so use pointer
    XDecoder(const doc_type *parent, const char* key) {
        init_base(parent, key);
    }
    XDecoder(const doc_type *parent, size_t index) {
        init_base(parent, index);
    }
    ~XDecoder(){}

public:
    // for array
    template <class T, size_t N>
    inline bool decode(const char*key, T (&val)[N], const Extend *ext) {
        return this->decode(key, val, N, ext);
    }

    template <class T>
    bool decode(const char *key, T *val, size_t N, const Extend *ext) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        size_t mx = obj->Size();
        mx = mx>N?N:mx;

        doc_type sub;
        for (size_t i=0; i<mx; ++i) {
            obj->member(i, sub).decode(NULL, val[i], ext);
        }
        return true;
    }
    // char[] is special
    bool decode(const char*key, char* val, size_t N, const Extend *ext) {
        std::string str;
        bool ret = ((doc_type*)this)->decode(key, str, ext);
        if (ret) {
            size_t mx = str.length();
            mx = mx>N-1?N-1:mx;
            strncpy(val, str.data(), mx);
            val[mx] = '\0';
        }
        return ret;
    }

    // vector
    template <class T>
    inline bool decode(const char*key, std::vector<T> &val, const Extend *ext) {
        return this->decode_vector(key, val, ext);
    }

    // list
    template <class T>
    bool decode(const char*key, std::list<T> &val, const Extend *ext) {
        return this->decode_list<std::list<T>, T>(key, val, ext);
    }

    // set
    template <class T>
    bool decode(const char*key, std::set<T> &val, const Extend *ext) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        doc_type sub;
        size_t s = obj->Size();
        for (size_t i=0; i<s; ++i) {
            T _t;
            obj->member(i, sub).decode(NULL, _t, ext);
            val.insert(_t);
        }
        return true;
    }

    // map
    template <class T>
    bool decode(const char*key, std::map<std::string,T> &val, const Extend *ext) {
        return decode_map<std::map<std::string,T>, std::string, T>(key, val, ext, KeyConverter::toString);
    }

    // class/struct that defined macro XPACK, !is_xpack_out to avoid inherit __x_pack_value
    template <class T>
    typename x_enable_if<T::__x_pack_value&&!is_xpack_out<T>::value, bool>::type decode(const char*key, T& val, const Extend *ext) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        val.__x_pack_decode(*obj, val, ext);
        return true;
    }

    // class/struct that defined macro XPACK_OUT
    template <class T>
    typename x_enable_if<is_xpack_out<T>::value, bool>::type decode(const char*key, T& val, const Extend *ext) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        __x_pack_decode_out(*obj, val, ext);
        return true;
    }

    // XType. add && !is_xpack_out<T>::value to fix SFINAE bug of vs2005 
    template <class T>
    inline typename x_enable_if<is_xpack_xtype<T>::value && !is_xpack_out<T>::value, bool>::type decode(const char*key, T& val, const Extend *ext) {
        return xpack_xtype_decode(*(doc_type*)this, key, val, ext);
    }

    #ifdef X_PACK_SUPPORT_CXX0X
    // unordered_map
    template <class T>
    inline bool decode(const char*key, std::unordered_map<std::string, T> &val, const Extend *ext) {
        return decode_map<std::unordered_map<std::string,T>, std::string, T>(key, val, ext, KeyConverter::toString);
    }

    // shared_ptr
    template <class T>
    bool decode(const char*key, std::shared_ptr<T>& val, const Extend *ext) {
        if (NULL == val.get()) {
            val.reset(new T);
        }
        bool ret = ((doc_type*)this)->decode(key, *val, ext);
        if (!ret) {
            val.reset();
        }
        return ret;
    }

    // enum is_enum implementation is too complicated, so in c++03, we use macro E
    template <class T>
    inline typename x_enable_if<std::is_enum<T>::value, bool>::type  decode(const char*key, T& val, const Extend *ext) {
        return ((doc_type*)this)->decode(key, *((int*)&val), ext);
    }
    #endif

    #ifdef XPACK_SUPPORT_QT
    bool decode(const char*key, QString &val, const Extend *ext) {
        std::string str;
        bool ret = ((doc_type*)this)->decode(key, str, ext);
        if (ret) {
            val = QString::fromStdString(str);
        }
        return ret;
    }

    template<typename T>
    inline bool decode(const char*key, QList<T> &val, const Extend *ext) {
        return this->decode_list<QList<T>, T>(key, val, ext);
    }

    template<typename T>
    inline bool decode(const char*key, QVector<T> &val, const Extend *ext) {
        return this->decode_vector(key, val, ext);
    }

    template<typename T>
    inline bool decode(const char*key, QMap<std::string, T> &val, const Extend *ext) {
        return decode_map<QMap<std::string,T>, std::string, T>(key, val, ext, KeyConverter::toString);
    }

    template<typename T>
    inline bool decode(const char*key, QMap<QString, T> &val, const Extend *ext) {
        return decode_map<QMap<QString,T>, QString, T>(key, val, ext, KeyConverter::toQString);
    }
    #endif
protected:
    // vector
    template <class Vector>
    bool decode_vector(const char*key, Vector &val, const Extend *ext) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        doc_type sub;
        size_t s = obj->Size();
        val.resize(s);
        for (size_t i=0; i<s; ++i) {
            obj->member(i, sub).decode(NULL, val[i], ext);
        }
        return true;
    }
    // list
    template <class List, class Elem>
    bool decode_list(const char*key, List &val, const Extend *ext) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        doc_type sub;
        size_t s = obj->Size();
        for (size_t i=0; i<s; ++i) {
            Elem _t;
            obj->member(i, sub).decode(NULL, _t, ext);
            val.push_back(_t);
        }
        return true;
    }
    // map
    template <class Map, class Key, class Value>
    bool decode_map(const char*key, Map &val, const Extend *ext, bool (*convert)(const char*, Key&)) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        doc_type sub;
        for (typename doc_type::Iterator d=obj->Begin(); d!=obj->End(); ++d) {
            Key   _k;
            Value _t;
            if (convert(d.Key(), _k) && obj->member(d, sub).decode(NULL, _t, ext)) {
                val[_k] = _t;
            }
        }

        return true;
    }

protected:
    doc_type* find(const char *key, doc_type *tmp, const Extend *ext) {
        doc_type *obj = static_cast<doc_type*>(this);
        if (NULL != key) {
            if (!obj->member(key, *tmp)) {
                if (Extend::Mandatory(ext)) {
                    decode_exception("mandatory key not found", key);
                } else {
                    return NULL;
                }
            } else {
                return tmp;
            }
        } else {
            return obj;
        }
        return NULL; // for remove warning
    }

    std::string path() const {
        std::vector<std::string> nodes;
        const doc_type* tmp = static_cast<const doc_type*>(this);
        while (tmp) {
            std::string k;
            k.reserve(32);
            if (NULL != tmp->_key) {
                if (NULL!=tmp->_parent && NULL!=tmp->_parent->_parent) {
                    k.append(".");
                }
                k.append(tmp->_key);
            } else {
                k.append("[").append(Util::itoa(tmp->_index)).append("]");
            }
            nodes.push_back(k);
            tmp = tmp->_parent;
        }
        std::string p;
        p.reserve(64);
        for (int i=(int)nodes.size()-1; i>=0; --i) {
            p.append(nodes[i]);
        }
        return p;
    }
    void decode_exception(const char* what, const char *key) const {
        std::string err;
        err.reserve(128);
        if (NULL != what) {
            err.append(what);
        }
        err.append("[");
        std::string p = path();
        if (!p.empty()) {
            err.append(p).append(".");
        }
        if (NULL != key) {
            err.append(key);
        }
        err.append("]");
        throw std::runtime_error(err);
    }

    doc_type* alloc() {
        doc_type *d = new doc_type();
        _collector.push_back(d);
        return d;
    }

    void init_base(const doc_type *parent, const char* key) {
        _parent = parent;
        _key = key;
        _index = -1;
    }
    void init_base(const doc_type *parent, size_t index) {
        _parent = parent;
        _index = (int)index;
        _key = NULL;
    }

    const doc_type* _parent;
    const char* _key;
    int _index;

    std::vector<doc_type*> _collector;
};

}

#endif
