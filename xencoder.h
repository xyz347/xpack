﻿/*
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


#ifndef __X_PACK_ENCODER_H
#define __X_PACK_ENCODER_H

#include <map>
#include <set>
#include <vector>
#include <list>

#include "extend.h"
#include "traits.h"
#include "numeric.h"
#include "xtype.h"

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

    ArrayBegin(const char *key, const Extend *ext)  begin encode array
    ArrayEnd(const char *key, const Extend *ext)    end encode array

    ObjectBegin(const char *key, const Extend *ext) begin encode object
    ObjectEnd(const char *key, const Extend *ext)   end encode object
    writeNull(const char *key, const Extend *ext)
*/
template<typename DOC>
class XEncoder {
protected:
    typedef DOC doc_type;
    typedef XEncoder<DOC> xdoc_type;
public:
    // for array
    template <class T, size_t N>
    inline bool encode(const char*key, const T (&val)[N], const Extend *ext) {
        return this->encode(key, val, N, ext);
    }

    template <class T>
    bool encode(const char *key, const T *val, size_t N, const Extend *ext) {
        if (N==0 && Extend::OmitEmpty(ext)) {
            return false;
        }

        doc_type *dt = (doc_type*)this;
        dt->ArrayBegin(key, ext);
        for (size_t i=0; i<N; ++i) {
            dt->encode(dt->IndexKey(i), val[i], ext);
        }
        dt->ArrayEnd(key, ext);
        return true;
    }
    bool encode(const char*key, const char *val, size_t N, const Extend *ext) {
        (void)N;
        std::string str(val);
        return ((doc_type*)this)->encode(key, str, ext);
    }

    // vector
    template <class T>
    bool encode(const char*key, const std::vector<T> &val, const Extend *ext) {
        size_t s = val.size();
        if (s==0 && Extend::OmitEmpty(ext)) {
            return false;
        }

        doc_type *dt = (doc_type*)this;
        dt->ArrayBegin(key, ext);
        for (size_t i=0; i<s; ++i) {
            dt->encode(dt->IndexKey(i), val[i], ext);
        }
        dt->ArrayEnd(key, ext);
        return true;
    }

    // list
    template <class T>
    bool encode(const char*key, const std::list<T> &val, const Extend *ext) {
        size_t s = val.size();
        if (s==0 && Extend::OmitEmpty(ext)) {
            return false;
        }

        doc_type *dt = (doc_type*)this;
        dt->ArrayBegin(key, ext);
        size_t i = 0;
        for (typename std::list<T>::const_iterator it=val.begin(); it!=val.end(); ++it, ++i) {
            dt->encode(dt->IndexKey(i), *it, ext);
        }
        dt->ArrayEnd(key, ext);
        return true;
    }

    // set
    template <class T>
    bool encode(const char*key, const std::set<T> &val, const Extend *ext) {
        size_t s = val.size();
        if (s==0 && Extend::OmitEmpty(ext)) {
            return false;
        }

        doc_type *dt = (doc_type*)this;
        dt->ArrayBegin(key, ext);
        size_t i = 0;
        for (typename std::set<T>::const_iterator it=val.begin(); it!=val.end(); ++it, ++i) {
            dt->encode(dt->IndexKey(i), *it, ext);
        }
        dt->ArrayEnd(key, ext);
        return true;
    }

    // map
    template <class T>
    bool encode(const char*key, const std::map<std::string, T> &val, const Extend *ext) {
        size_t s = val.size();
        if (s==0 && Extend::OmitEmpty(ext)) {
            return false;
        }

        doc_type *dt = (doc_type*)this;
        dt->ObjectBegin(key, ext);
        for (typename std::map<std::string, T>::const_iterator it=val.begin(); it!=val.end(); ++it) {
            dt->encode(it->first.c_str(), it->second, ext);
        }
        dt->ObjectEnd(key, ext);
        return true;
    }

    // class/struct that defined macro XPACK
    template <class T>
    typename x_enable_if<T::__x_pack_value&&!is_xpack_out<T>::value, bool>::type encode(const char*key, const T& val, const Extend *ext) {
        bool inherit = 0!=(X_PACK_CTRL_FLAG_INHERIT&Extend::CtrlFlag(ext));
        doc_type *dt = (doc_type*)this;
        if (!inherit) {
            dt->ObjectBegin(key, ext);
        }
        val.__x_pack_encode(*dt, val, ext);
        if (!inherit) {
            dt->ObjectEnd(key, ext);
        }
        return true;
    }

    // class/struct that defined macro XPACK_OUT
    template <class T>
    typename x_enable_if<is_xpack_out<T>::value, bool>::type encode(const char*key, const T& val, const Extend *ext) {
        bool inherit = 0!=(X_PACK_CTRL_FLAG_INHERIT&Extend::CtrlFlag(ext));
        doc_type *dt = (doc_type*)this;
        if (!inherit) {
           dt->ObjectBegin(key, ext);
        }
        __x_pack_encode_out(*dt, val, ext);
        if (!inherit) {
            dt->ObjectEnd(key, ext);
        }
        return true;
    }

    // XType. add && !is_xpack_out<T>::value to fix SFINAE bug of vs2005
    template <class T>
    typename x_enable_if<is_xpack_xtype<T>::value && !is_xpack_out<T>::value, bool>::type encode(const char*key, const T& val, const Extend *ext) {
        return xpack_xtype_encode(*(doc_type*)this, key, val, ext);
    }

    #ifdef X_PACK_SUPPORT_CXX0X
    // unordered_map
    template <class T>
    bool encode(const char*key, const std::unordered_map<std::string, T> &val, const Extend *ext) {
        size_t s = val.size();
        if (s==0 && Extend::OmitEmpty(ext)) {
            return false;
        }

        doc_type *dt = (doc_type*)this;
        dt->ObjectBegin(key, ext);
        for (typename std::unordered_map<std::string, T>::const_iterator it=val.begin(); it!=val.end(); ++it) {
            dt->encode(it->first.c_str(), it->second, ext);
        }
        dt->ObjectEnd(key, ext);
        return true;
    }

    // shared_ptr
    template <class T>
    bool encode(const char*key, const std::shared_ptr<T>& val, const Extend *ext) {
        if (val.get() == NULL) { // if shared ptr is null
            return ((doc_type*)this)->writeNull(key, ext);
        }

        return ((doc_type*)this)->encode(key, *val, ext);
    }

    // enum is_enum implementation is too complicated, so in c++03, we use macro E
    template <class T>
    typename x_enable_if<std::is_enum<T>::value, bool>::type  decode(const char*key, const T& val, const Extend *ext) {
        return ((doc_type*)this)->encode(key, (const int&)val, ext);
    }
    #endif // cxx

    #ifdef XPACK_SUPPORT_QT
    bool encode(const char*key, const QString &val, const Extend *ext) {
        std::string str = val.toStdString();
        return ((doc_type*)this)->encode(key, str, ext);
    }

    template<typename T>
    bool encode(const char*key, const QList<T>&data, const Extend *ext) {
        std::list<T> sl = std::list<T>(data.begin(), data.end());
        return ((doc_type*)this)->encode(key, sl, ext);
    }

    template<typename T>
    bool encode(const char*key, const QMap<std::string, T>&data, const Extend *ext) {
        std::map<std::string, T> sm = data.toStdMap();
        return ((doc_type*)this)->encode(key, sm, ext);
    }

    template<typename T>
    bool encode(const char*key, const QMap<QString, T>&data, const Extend *ext) {
        std::map<std::string, T> sm;
        for (typename QMap<QString, T>::const_iterator iter=data.begin(); iter!=data.end(); ++iter) {
            sm[iter.key().toStdString()] = iter.value();
        }
        return ((doc_type*)this)->encode(key, sm, ext);
    }

    template<typename T>
    bool encode(const char*key, const QVector<T>&data, const Extend *ext) {
        std::vector<T> sv = data.toStdVector();
        return ((doc_type*)this)->encode(key, sv, ext);
    }
    #endif

    // wrapper for encode manual
    template <class T>
    doc_type& add(const char *key, const T&val, const Extend *ext=NULL) {
        doc_type *dt = (doc_type*)this;
        dt->encode(key, val, ext);
        return *dt;
    }

    // object begin
    doc_type& ob(const char *key) {
        doc_type *dt = (doc_type*)this;
        dt->ObjectBegin(key, NULL);
        return *dt;
    }

    // object end
    doc_type& oe(const char *key=NULL) {
        doc_type *dt = (doc_type*)this;
        dt->ObjectEnd(key, NULL);
        return *dt;
    }

    // array begin
    doc_type& ab(const char *key) {
        doc_type *dt = (doc_type*)this;
        dt->ArrayBegin(key, NULL);
        return *dt;
    }

    // array end
    doc_type& ae(const char *key=NULL) {
        doc_type *dt = (doc_type*)this;
        dt->ArrayEnd(key, NULL);
        return *dt;
    }
};

}

#endif
