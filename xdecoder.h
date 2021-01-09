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

#include "traits.h"
#include "xtype.h"

#ifdef XPACK_SUPPORT_CHAR_ARRAY
#include "string.h"
#endif

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

#include "extend.h"

namespace xpack {


/*
  DOC need implement:
    const char *Type() const; "json/bson/...."

    DOC *Find(const char *key, DOC *tmp) const; find obj by key;

    DOC Begin(); return iter;
    DOC Next();  iter next;
    operator bool;

    size_t Size() const; if doc is array, return array size;
    DOC *At(size_t) get member of array;
*/
template<class DOC>
class XDecoder {
protected:
    typedef DOC doc_type;
    typedef XDecoder<DOC> xdoc_type;

public:
    // only c++0x support reference initialize, so use pointer
    XDecoder(const doc_type *parent, const char* key):_parent(parent), _key(key), _index(-1) {
    }
    XDecoder(const doc_type *parent, size_t index):_parent(parent), _key(NULL), _index(int(index)) {
    }
    ~XDecoder(){}

public:
    // vector
    template <class T>
    bool decode(const char*key, std::vector<T> &val, const Extend *ext) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        size_t s = obj->Size();
        val.resize(s);
        for (size_t i=0; i<s; ++i) {
            obj->At(i).decode(NULL, val[i], ext);
        }
        return true;
    }

    // list
    template <class T>
    bool decode(const char*key, std::list<T> &val, const Extend *ext) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        size_t s = obj->Size();
        for (size_t i=0; i<s; ++i) {
            T _t;
            obj->At(i).decode(NULL, _t, ext);
            val.push_back(_t);
        }
        return true;
    }

    // set
    template <class T>
    bool decode(const char*key, std::set<T> &val, const Extend *ext) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        size_t s = obj->Size();
        for (size_t i=0; i<s; ++i) {
            T _t;
            obj->At(i).decode(NULL, _t, ext);
            val.insert(_t);
        }
        return true;
    }

    // map
    template <class T>
    bool decode(const char*key, std::map<std::string,T> &val, const Extend *ext) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        for (doc_type d=obj->Begin(); d; d=d.Next()) {
            T _t;
            d.decode(NULL, _t, ext);
            val[d._key] = _t;
        }
        return true;
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

    // XType 
    template <class T>
    typename x_enable_if<is_xpack_xtype<T>::value, bool>::type decode(const char*key, T& val, const Extend *ext) {
        return xpack_xtype_decode(*(doc_type*)this, key, val, ext);
    }

    #ifdef X_PACK_SUPPORT_CXX0X
    // unordered_map
    template <class T>
    bool decode(const char*key, std::unordered_map<std::string, T> &val, const Extend *ext) {
        doc_type tmp;
        doc_type *obj = find(key, &tmp, ext);
        if (NULL == obj) {
            return false;
        }

        for (doc_type d=obj->Begin(); d; d=d.Next()) {
            T _t;
            d.decode(NULL, _t, ext);
            val[d._key] = _t;
        }
        return true;
    }

    // shared_ptr
    template <class T>
    bool decode(const char*key, std::shared_ptr<T>& val, const Extend *ext) {
        if (NULL == val.get()) {
            val.reset(new T);
        }
        return ((doc_type*)this)->decode(key, *val, ext);
    }

    // enum is_enum implementation is too complicated, so in c++03, we use macro E
    template <class T>
    typename x_enable_if<std::is_enum<T>::value, bool>::type  decode(const char*key, T& val, const Extend *ext) {
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
    bool decode(const char*key, QList<T> &val, const Extend *ext) {
        std::list<T> sl;
        bool ret = ((doc_type*)this)->decode(key, sl, ext);
        if (ret) {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
            val = QList<T>::fromStdList(sl);
#else
            val = QList<T>(sl.begin(), sl.end());
#endif
        }
        return ret;
    }

    template<typename T>
    bool decode(const char*key, QVector<T> &val, const Extend *ext) {
        std::vector<T> sv;
        bool ret = ((doc_type*)this)->decode(key, sv, ext);
        if (ret) {
            val = QVector<T>::fromStdVector(sv);
        }
        return ret;
    }

    template<typename T>
    bool decode(const char*key, QMap<std::string, T> &val, const Extend *ext) {
        std::map<std::string, T> sm;
        bool ret = ((doc_type*)this)->decode(key, sm, ext);
        if (ret) {
            val = QMap<std::string, T>(sm);
        }
        return ret;
    }

    template<typename T>
    bool decode(const char*key, QMap<QString, T> &val, const Extend *ext) {
        std::map<std::string, T> sm;
        bool ret = ((doc_type*)this)->decode(key, sm, ext);
        if (ret) {
            for (typename std::map<std::string, T>::const_iterator iter = sm.begin(); iter!=sm.end(); iter++) {
                val[QString::fromStdString(iter->first)] = iter->second;
            }
        }
        return ret;
    }
    #endif

    #ifdef XPACK_SUPPORT_CHAR_ARRAY
    bool decode(const char*key, char val[], const Extend *ext) {
        std::string str;
        bool ret = ((doc_type*)this)->decode(key, str, ext);
        if (ret) {
            strncpy(val, str.data(), str.length());
            val[str.length()] = '\0';
        }
        return ret;
    }
    #endif

protected:
    doc_type* find(const char *key, doc_type *tmp, const Extend *ext) {
        doc_type *obj = static_cast<doc_type*>(this);
        if (NULL != key) {
            obj = obj->Find(key, tmp);
            if (NULL == obj && Extend::Mandatory(ext)) {
                decode_exception("mandatory key not found", key);
            }
        }
        return obj;
    }

    std::string path() {
        std::vector<std::string> nodes;
        const doc_type* tmp = static_cast<doc_type*>(this);
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
    void decode_exception(const char* what, const char *key) {
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

    const doc_type* _parent;
    const char* _key;
    int _index;
};

}

#endif
