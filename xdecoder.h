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
#include <type_traits>
#endif

namespace xpack {


/*
  Node need implement:
    static const char *Type() const; "json/bson/...."

    operator bool() const;          // check is a valid node
    bool IsNull() const;            // only for json, others should always return false
    Node Find(decoder&, const char*key);      // Find a child node(object)
    Node At(size_t index);          // Find a child node(array)
    size_t Size(decoder&, );        // Get array size

    // iterator
    Node Next(decoder&, const Node&p, Iterator&iter, std::string&key);       // iter object

    template <class T>
    bool Get(decoder&, T&val, const Extend*ext); // T is integer/double/std::string/bool
*/
template<class Node>
class XDecoder {
public:
    typedef XDecoder<Node> decoder;

    XDecoder(const decoder* parent, const char* key, Node node):_p(parent),_k(key),_i(-1),_n(node) {}
    XDecoder(const decoder *parent, int index, Node node):_p(parent),_k(NULL),_i(index),_n(node) {}
    XDecoder():_i(-2){}

    const char *Name() const {
        return Node::Name();
    }
    void decode_exception(const char* what, const char *key) const {
        std::string err;
        err.reserve(128);
        if (NULL != what) {
            err.append(what);
        }
        err.append(". (path:");

        std::string p = path();
        err.append(p);
        if (NULL != key) {
            if (!p.empty()) {
                err.append(".");
            }
            err.append(key);
        }

        err.append(")");
        throw std::runtime_error(err);
    }
    // find by key
    decoder Find(const char *key, const Extend *ext) {
        Node child = _n.Find(*this, key, ext);
        if (child){
            return XDecoder(this, key, child);
        } else if (Extend::Mandatory(ext)) {
            decode_exception("mandatory key not found", key);
        }

        return XDecoder();
    }
    operator bool() const {
        return _i != -2;
    }

public:
    template <class T>
    bool decode(const char*key, T&val, const Extend*ext) {
        decoder child = Find(key, ext);
        if (child) {
            return child.decode_type(val, ext);
        }
        return false;
    }

    template <class T>
    bool decode(T &val, const Extend*ext) {
        return this->decode_type(val, ext);
    }

    // class/struct that defined macro XPACK, !is_xpack_out to avoid inherit __x_pack_value
    template <class T>
    inline typename x_enable_if<T::__x_pack_value && !is_xpack_out<T>::value, bool>::type decode_struct(T& val, const Extend *ext) {
        return val.__x_pack_decode(*this, val, ext);
    }
    // class/struct that defined macro XPACK_OUT
    template <class T>
    inline typename x_enable_if<is_xpack_out<T>::value, bool>::type decode_struct(T& val, const Extend *ext) {
        return __x_pack_decode_out(*this, val, ext);
    }

    template <class T>
    inline bool decode_struct(const char*key, T&val, const Extend *ext) {
        decoder child = Find(key, ext);
        if (child) {
            return child.decode_struct(val, ext);
        }
        return false;
    }

private:
    // numeric
    template <class T>
    inline typename x_enable_if<numeric<T>::value, bool>::type decode_type(T&val, const Extend *ext) {
        return _n.Get(*this, val, ext);
    }
    // std::string
    inline bool decode_type(std::string&val, const Extend *ext) {
        return _n.Get(*this, val, ext);
    }
    // std::string
    inline bool decode_type(bool&val, const Extend *ext) {
        return _n.Get(*this, val, ext);
    }
    // array
    template <class T, size_t N>
    inline bool decode_type(T (&val)[N], const Extend *ext) {
        return this->decode_array(val, N, ext);
    }
    // vector
    template <class T>
    inline bool decode_type(std::vector<T> &val, const Extend *ext) {
        return this->decode_vector(val, ext);
    }
    // list
    template <class T>
    inline bool decode_type(std::list<T> &val, const Extend *ext) {
        return this->decode_list<std::list<T>, T>(val, ext);
    }
    // set
    template <class T>
    inline bool decode_type(std::set<T> &val, const Extend *ext) {
        return this->decode_list<std::set<T>, T>(val, ext);
    }
    // map
    template <class K, class V>
    inline bool decode_type(std::map<K, V> &val, const Extend *ext) {
        return decode_map<std::map<K, V>, K, V>(val, ext);
    }
    // XPACK or XPACK_OUT and not XTYPE
    template <class T>
    inline XPACK_IS_XOUT(T) decode_type(T&val, const Extend *ext) {
        return decode_struct(val, ext);
    }
    template <class T>
    inline XPACK_IS_XPACK(T) decode_type(T&val, const Extend *ext) {
        return decode_struct(val, ext);
    }
    // xtype. add && !is_xpack_out<T>::value to fix SFINAE bug of vs2005 
    template <class T>
    inline XPACK_IS_XTYPE(Node, T) decode_type(T& val, const Extend *ext) {
        return xpack_xtype_decode(*this, val, ext);
    }

    #ifdef X_PACK_SUPPORT_CXX0X
    // unordered_map
    template <class K, class V>
    inline bool decode_type(std::unordered_map<K, V> &val, const Extend *ext) {
        return decode_map<std::unordered_map<K, V>, K, V>(val, ext);
    }
    // shared_ptr
    template <class T>
    bool decode_type(std::shared_ptr<T>& val, const Extend *ext) {
        bool ret = false;
        if (!_n.IsNull()) {
            val.reset(new T);
            ret = this->decode_type(*val, ext);
            if (!ret) {
                val.reset();
            }
        }
        return ret;
    }
    // enum is_enum implementation is too complicated, so in c++03, we use macro E
    template <class T>
    inline typename x_enable_if<std::is_enum<T>::value && !is_xpack_xtype<T>::value, bool>::type decode_type(T& val, const Extend *ext) {
        typename std::underlying_type<T>::type tmp;
        bool ret = this->decode_type(tmp, ext);
        if (ret) {
            val = (T)tmp;
        }
        return ret;
    }
    // assert pointer
    template <class T>
    typename x_enable_if<std::is_pointer<T>::value, bool>::type decode_type(T &val, const Extend *ext) {
        static_assert(!std::is_pointer<T>::value, "not support pointer, use shared_ptr please");
        (void)val;(void)ext;
        return false;
    }
    #endif

    //////////////////// QT ///////////////////////////////
    #ifdef XPACK_SUPPORT_QT
    bool decode_type(QString &val, const Extend *ext) {
        std::string str;
        bool ret = this->decode_type(str, ext);
        if (ret) {
            val = QString::fromStdString(str);
        }
        return ret;
    }
    template<typename T>
    inline bool decode_type(QList<T> &val, const Extend *ext) {
        return this->decode_list<QList<T>, T>(val, ext);
    }
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) //https://www.qt.io/blog/qlist-changes-in-qt-6
    template<typename T>
    inline bool decode_type(QVector<T> &val, const Extend *ext) {
        return this->decode_vector(val, ext);
    }
    #endif

    template<typename K, typename V>
    inline bool decode_type(QMap<K, V> &val, const Extend *ext) {
        return decode_map<QMap<K, V>, K, V>(val, ext);
    }
    #endif

    template <typename T>
    inline typename x_enable_if<is_xpack_type_spec<Node, T>::value, bool>::type decode_type(T&val, const Extend *ext) {
        return _n.decode_type_spec(val, ext);
    }


    ///////////////////////////////////////////////////
    template <class T>
    bool decode_array(T *val, size_t N, const Extend *ext) {
        size_t mx = _n.Size(*this);
        mx = mx>N?N:mx;

        for (size_t i=0; i<mx; ++i) {
            this->at(i, ext).decode_type(val[i], ext);
        }
        return true;
    }
    // char[] is special
    bool decode_array(char* val, size_t N, const Extend *ext) {
        std::string str;
        bool ret = this->decode_type(str, ext);
        if (ret) {
            size_t mx = str.length();
            mx = mx>N-1?N-1:mx;
            strncpy(val, str.data(), mx);
            val[mx] = '\0';
        }
        return ret;
    }
    template <class Vector>
    bool decode_vector(Vector &val, const Extend *ext) {
        size_t s = _n.Size(*this);
        val.resize(s);
        for (size_t i=0; i<s; ++i) {
            this->at(i, ext).decode_type(val[i], ext);
        }
        return true;
    }
    // list
    template <class List, class Elem>
    bool decode_list(List &val, const Extend *ext) {
        size_t s = _n.Size(*this);
        for (size_t i=0; i<s; ++i) {
            Elem _t;
            this->at(i, ext).decode_type(_t, ext);
            this->add_ele(val, _t);
        }
        return true;
    }
    // map
    template <class Map, class K, class V>
    bool decode_map(Map &val, const Extend *ext) {
        typename Node::Iterator iter;
        std::string key;
        Node tmp = _n.Next(*this, _n, iter, key);
        while (tmp) {
            K k;
            V v;
            if (keyConvert(key, k) && XDecoder(this, key.c_str(), tmp).decode_type(v, ext)) {
                val[k] = v;
            }
            tmp = tmp.Next(*this, _n, iter, key);
        }

        return true;
    }

    ////// container process //////
    template <class T>
    inline void add_ele(std::list<T>&val, T &t) {
        val.push_back(t);
    }
    template <class T>
    inline void add_ele(std::set<T>&val, T &t) {
        val.insert(t);
    }
    #ifdef XPACK_SUPPORT_QT
    template <class T>
    inline void add_ele(QList<T>&val, T &t) {
        val.push_back(t);
    }
    #endif

    // convert map key
    inline bool keyConvert(std::string&s, std::string&key) {
        key.swap(s);
        return true;
    }
    #ifdef XPACK_SUPPORT_QT
    inline bool keyConvert(std::string&s, QString &key) {
        key = QString::fromStdString(s);
        return true;
    }
    #endif
    #ifdef X_PACK_SUPPORT_CXX0X
    // enum is_enum implementation is too complicated, so not support in c++03
    template <class T>
    typename x_enable_if<std::is_enum<T>::value, bool>::type keyConvert(std::string&s, T&key) {
        typename std::underlying_type<T>::type tmp;
        bool ret = Util::atoi(s, tmp);
        if (ret) {
            key = (T)tmp;
        }
        return ret;
    }
    #endif
    template <class T>
    inline typename x_enable_if<numeric<T>::is_integer, bool>::type keyConvert(std::string&s, T&key) {
        return Util::atoi(s, key);
    }

    // index
    decoder at(size_t i, const Extend *ext) {
        (void)ext;
        return XDecoder(this, (int)i, _n.At(i));
    }

    // exception process
    std::string path() const {
        std::vector<std::string> nodes;
        const decoder* tmp = this;
        while (tmp) {
            std::string k;
            k.reserve(32);
            if (NULL != tmp->_k) {
                if (NULL!=tmp->_p && NULL!=tmp->_p->_p) {
                    k.append(".");
                }
                k.append(tmp->_k);
            } else if (_i >= 0) {
                k.append("[").append(Util::itoa(tmp->_i)).append("]");
            }
            nodes.push_back(k);
            tmp = tmp->_p;
        }
        std::string p;
        p.reserve(64);
        for (int i=(int)nodes.size()-1; i>=0; --i) {
            p.append(nodes[i]);
        }
        return p;
    }

    const decoder* _p;
    const char* _k;
    int _i;
    Node _n;
};

}

#endif
