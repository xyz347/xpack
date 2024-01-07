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


#ifndef __X_PACK_ENCODER_H
#define __X_PACK_ENCODER_H

#include <map>
#include <set>
#include <vector>
#include <list>

#include "extend.h"
#include "traits.h"
#include "numeric.h"

#ifdef XPACK_SUPPORT_QT
#include <QString>
#include <QList>
#include <QMap>
#include <QVector>
#endif

#ifdef X_PACK_SUPPORT_CXX0X
#include <memory>
#include <unordered_map>
#include <type_traits>
#endif

namespace xpack {

/*
  DOC need implement:
    const char *Name() const; "json/bson/...."
    const static bool support_null = true/false;

    ArrayBegin(const char *key, const Extend *ext)  begin encode array
    ArrayEnd(const char *key, const Extend *ext)    end encode array

    ObjectBegin(const char *key, const Extend *ext) begin encode object
    ObjectEnd(const char *key, const Extend *ext)   end encode object

    WriteNull(const char *key, const Extend *ext)

    std::string String() return the encode result

    // basic type
    bool encode_bool(const char*key, const bool&val, const Extend *ext);
    bool encode_string(const char*key, const std::string&val, const Extend *ext);
    bool encode_number(const char*key, const T&val, const Extend *ext);
*/
template<typename Writer>
class XEncoder :private noncopyable{
    typedef XEncoder<Writer> encoder;
    Writer &_w;

public:
    XEncoder(Writer &w):_w(w){}

    const char *Name() const {
        return Writer::Name();
    }

    // After calling String, this Encoder has finished its work and should not be used anymore
    inline std::string String() {
        return _w.String();
    }

    #define XPACK_WRITE_EMPTY(cond) \
    if (cond) { \
        if (Extend::OmitEmpty(ext)) { \
            return false;\
        } else if (Writer::support_null && Extend::EmptyNull(ext)) {\
           return _w.WriteNull(key, ext);\
        }\
    }

    // basic type
    bool encode(const char*key, const bool &val, const Extend *ext) {
        XPACK_WRITE_EMPTY(!val);
        return _w.encode_bool(key, val, ext);
    }
    bool encode(const char*key, const std::string &val, const Extend *ext) {
        XPACK_WRITE_EMPTY(val.empty());
        return _w.encode_string(key, val, ext);
    }
    template <class T>
    typename x_enable_if<numeric<T>::value, bool>::type encode(const char*key, const T&val, const Extend *ext) {
        XPACK_WRITE_EMPTY(val == 0);
        return _w.encode_number(key, val, ext);
    }

    // array
    template <class T, size_t N>
    inline bool encode(const char*key, const T (&val)[N], const Extend *ext) {
        return this->encode(key, val, N, ext);
    }
    template <class T>
    bool encode(const char *key, const T *val, size_t N, const Extend *ext) {
        XPACK_WRITE_EMPTY((N==0))

        _w.ArrayBegin(key, ext);
        for (size_t i=0; i<N; ++i) {
            this->encode(_w.IndexKey(i), val[i], NULL);
        }
        _w.ArrayEnd(key, ext);
        return true;
    }
    inline bool encode(const char*key, const char *val, size_t N, const Extend *ext) {
        (void)N;
        std::string str(val);
        return this->encode(key, str, ext);
    }
    // vector
    template <class T>
    inline bool encode(const char*key, const std::vector<T> &val, const Extend *ext) {
        return encode_list<std::vector<T> >(key, val, ext);
    }
    // list
    template <class T>
    inline bool encode(const char*key, const std::list<T> &val, const Extend *ext) {
        return encode_list<std::list<T> >(key, val, ext);
    }
    // set
    template <class T>
    inline bool encode(const char*key, const std::set<T> &val, const Extend *ext) {
        return encode_list<std::set<T> >(key, val, ext);
    }
    // map
    template <class K, class V>
    inline bool encode(const char*key, const std::map<K, V> &val, const Extend *ext) {
        return encode_map<const std::map<K, V> >(key, val, ext);
    }
    // class/struct that defined XPACK
    template <class T>
    inline XPACK_IS_XPACK(T) encode(const char*key, const T& val, const Extend *ext) {
        return encode_struct(key, val, ext);
    }
    // class/struct that defined XPACK_OUT
    template <class T>
    inline XPACK_IS_XOUT(T) encode(const char*key, const T& val, const Extend *ext) {
        return encode_struct(key, val, ext);
    }
    // xtype
    template <class T>
    inline XPACK_IS_XTYPE(Writer, T) encode(const char*key, const T& val, const Extend *ext) {
        return encode_xtype(key, val, ext);
    }

    #ifdef X_PACK_SUPPORT_CXX0X
    // unordered_map
    template <class K, class V>
    inline bool encode(const char*key, const std::unordered_map<K, V> &val, const Extend *ext) {
        return encode_map<const std::unordered_map<K, V>>(key, val, ext);
    }
    // shared_ptr
    template <class T>
    bool encode(const char*key, const std::shared_ptr<T>& val, const Extend *ext) {
        if (val.get() == NULL) { // if shared ptr is null
            return _w.WriteNull(key, ext);
        }

        return this->encode(key, *val, ext);
    }
    // enum
    template <class T>
    typename x_enable_if<std::is_enum<T>::value && !is_xpack_xtype<T>::value, bool>::type encode(const char*key, const T& val, const Extend *ext) {
        typename std::underlying_type<T>::type tmp = (typename std::underlying_type<T>::type)val;
        return this->encode(key, tmp, ext);
    }
    // assert pointer
    template <class T>
    typename x_enable_if<std::is_pointer<T>::value, bool>::type encode(const char*key, const T &val, const Extend *ext) {
        static_assert(!std::is_pointer<T>::value, "no support pointer, use shared_ptr please");
        (void)key;(void)val;(void)ext;
        return false;
    }
    #endif // cxx
    template <class T>
    inline bool encode_xtype(const char*key, const T &val, const Extend *ext) {
        return xpack_xtype_encode(*this, key, val, ext);
    }

    #ifdef XPACK_SUPPORT_QT
    bool encode(const char*key, const QString &val, const Extend *ext) {
        std::string str = val.toStdString();
        return this->encode(key, str, ext);
    }

    template<class T>
    bool encode(const char*key, const QList<T>&val, const Extend *ext) {
        return encode_list<QList<T> >(key, val, ext);
    }
    template<class K, class V>
    bool encode(const char*key, const QMap<K, V>&val, const Extend *ext) {
        return encode_qmap<const QMap<K, V> >(key, val, ext);
    }

    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) //https://www.qt.io/blog/qlist-changes-in-qt-6
    template<typename T>
    bool encode(const char*key, const QVector<T>&val, const Extend *ext) {
        return encode_list<QVector<T> >(key, val, ext);
    }
    #endif // Qt5

    #endif // Qt

    template <typename T>
    inline typename x_enable_if<is_xpack_type_spec<Writer, T>::value, bool>::type encode(const char*key, const T&val, const Extend *ext) {
        return _w.encode_type_spec(key, val, ext);
    }

    // only for class/struct that defined XPACK
    template <class T>
    typename x_enable_if<T::__x_pack_value && !is_xpack_out<T>::value, bool>::type encode_struct(const char*key, const T& val, const Extend *ext) {
        bool inherit = 0!=(X_PACK_CTRL_FLAG_INHERIT&Extend::CtrlFlag(ext));
        if (!inherit) {
            _w.ObjectBegin(key, ext);
        }
        bool ret = val.__x_pack_encode(*this, val, ext);
        if (!inherit) {
            _w.ObjectEnd(key, ext);
        }
        return ret;
    }
    // only for class/struct that defined XPACK_OUT
    template <class T>
    typename x_enable_if<is_xpack_out<T>::value, bool>::type encode_struct(const char*key, const T& val, const Extend *ext) {
        bool inherit = 0!=(X_PACK_CTRL_FLAG_INHERIT&Extend::CtrlFlag(ext));

        if (!inherit) {
           _w.ObjectBegin(key, ext);
        }
        bool ret = __x_pack_encode_out(*this, val, ext);
        if (!inherit) {
            _w.ObjectEnd(key, ext);
        }
        return ret;
    }

    // wrapper for encode manual
    inline void ArrayBegin(const char *key, const Extend *ext) {
        _w.ArrayBegin(key, ext);
    }
    inline void ArrayEnd(const char *key, const Extend *ext) {
        _w.ArrayEnd(key, ext);
    }
    inline void ObjectBegin(const char *key, const Extend *ext) {
        _w.ObjectBegin(key, ext);
    }
    inline void ObjectEnd(const char *key, const Extend *ext) {
        _w.ObjectEnd(key, ext);
    }
    inline void WriteNull(const char *key, const Extend *ext) {
        _w.WriteNull(key, ext);
    }
    template <class T>
    encoder& add(const char *key, const T&val, const Extend *ext=NULL) {
        this->encode(key, val, ext);
        return *this;
    }
    // object begin
    encoder& ob(const char *key) {
        this->ObjectBegin(key, NULL);
        return *this;
    }
    // object end
    encoder& oe(const char *key=NULL) {
        this->ObjectEnd(key, NULL);
        return *this;
    }

    // array begin
    encoder& ab(const char *key) {
        this->ArrayBegin(key, NULL);
        return *this;
    }
    // array end
    encoder& ae(const char *key=NULL) {
        this->ArrayEnd(key, NULL);
        return *this;
    }

private:

    // list
    template <class LIST>
    bool encode_list(const char*key, const LIST &val, const Extend *ext) {
        size_t s = val.size();
        XPACK_WRITE_EMPTY((s==0))

        _w.ArrayBegin(key, ext);
        size_t i = 0;
        for (typename LIST::const_iterator it=val.begin(); it!=val.end(); ++it, ++i) {
            this->encode(_w.IndexKey(i), *it, NULL);
        }
        _w.ArrayEnd(key, ext);
        return true;
    }

    // map
    template <class Map>
    bool encode_map(const char*key, Map &val, const Extend *ext) {
        size_t s = val.size();
        XPACK_WRITE_EMPTY((s==0))

        _w.ObjectBegin(key, ext);
        for (typename Map::const_iterator it=val.begin(); it!=val.end(); ++it) {
            this->encode(keyConvert(it->first).c_str(), it->second, NULL);
        }
        _w.ObjectEnd(key, ext);
        return true;
    }

    // qmap
    template <class Map>
    bool encode_qmap(const char*key, Map &val, const Extend *ext) {
        size_t s = val.size();
        XPACK_WRITE_EMPTY((s==0))

        _w.ObjectBegin(key, ext);
        for (typename Map::const_iterator it=val.begin(); it!=val.end(); ++it) {
            this->encode(keyConvert(it.key()).c_str(), it.value(), NULL);
        }
        _w.ObjectEnd(key, ext);
        return true;
    }

    // convert map key
    inline std::string keyConvert(const std::string&key) {
        return key;
    }
    #ifdef XPACK_SUPPORT_QT
    inline std::string keyConvert(const QString &key) {
        return key.toStdString();
    }
    #endif
    #ifdef X_PACK_SUPPORT_CXX0X
    // enum is_enum implementation is too complicated, so not support in c++03
    template <class T>
    typename x_enable_if<std::is_enum<T>::value, std::string>::type keyConvert(const T&key) {
        typename std::underlying_type<T>::type tmp = (typename std::underlying_type<T>::type)key;
        return Util::itoa(tmp);
    }
    #endif
    template <class T>
    inline typename x_enable_if<numeric<T>::is_integer, std::string>::type keyConvert(const T&key) {
        return Util::itoa(key);
    }

    #undef XPACK_WRITE_EMPTY
};

}

#endif
