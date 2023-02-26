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


#ifndef __X_PACK_MYSQL_DECODER_H
#define __X_PACK_MYSQL_DECODER_H

#include <stdexcept>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <cstdlib>

#include <mysql/mysql.h>

#include "extend.h"
#include "traits.h"
#include "json.h"

#include "string.h"
#include "time.h"

namespace xpack {

template <class T>
struct is_xpack_mysql_xtype {static bool const value = false;};
// if we will call xpack_mysql_decode(MYSQL_FIELD *field, char *row, T&val, const Extend *ext)

class MySQLDecoder {
    friend class mysql;
private:
    MYSQL_RES *_res;
    MYSQL_ROW _row;
    std::map<const char*, int, cmp_str> _index;

    MySQLDecoder(MYSQL_RES *result):_res(result), _row(NULL) {
        mysql_data_seek(_res, 0);
        for (int i=0; i<int(result->field_count); ++i) {
            _index[result->fields[i].name] = i;
        }
    }

    // class/struct that defined macro XPACK, !is_xpack_out to avoid inherit __x_pack_value
    template <class T>
    inline XPACK_IS_XPACK(T) decode_top(T& val, const Extend *ext) {
        if (NULL != (_row = mysql_fetch_row(_res))) {
            val.__x_pack_decode(*this, val, ext);
            return true;
        }
        return false;
    }
    template <class T>
    inline XPACK_IS_XPACK(T) decode_top(std::vector<T>& val, const Extend *ext) {
        while (NULL != (_row = mysql_fetch_row(_res))) {
            val.push_back(T());
            T &tmp = val.back();
            tmp.__x_pack_decode(*this, tmp, ext);
        }
        return true;
    }

    // class/struct that defined macro XPACK_OUT
    template <class T>
    inline XPACK_IS_XOUT(T) decode_top(std::vector<T>& val, const Extend *ext) {
        while (NULL != (_row = mysql_fetch_row(_res))) {
            val.push_back(T());
            __x_pack_decode_out(*this, val.back(), ext);
        }
        return true;
    }

    template <class T>
    bool decode_field(const char*field, T&val, const Extend *ext) {
        if (NULL==_row && NULL == (_row = mysql_fetch_row(_res))) {
            return false;
        }
        return decode(field, val, ext);
    }
    template <class T>
    bool decode_field(const char*field, std::vector<T>&val, const Extend *ext) {
        while (NULL != (_row = mysql_fetch_row(_res))) {
            T t;
            decode(field, t, ext);
            val.push_back(t);
        }
        return true;
    }


    int find(const char*field) const {
        std::map<const char*, int, cmp_str>::const_iterator it = _index.find(field);
        if (it != _index.end()) {
            return it->second;
        }
        return -1;
    }
public:
    const char *Type() const {
        return "db";
    }
    // std::string
    bool decode(const char*field, std::string &val, const Extend *ext) {
        int idx = find(field);
        if (idx >= 0) {
            val = _row[idx];
            return true;
        }
        return false;
    }
    // bool
    bool decode(const char*field, bool &val, const Extend *ext) {
        int tmp;
        if (decode(field, tmp, ext)) {
            val = tmp != 0;
            return true;
        }
        return false;
    }
    // unsigned integer. DATE and TIME use signed plz
    template <class T>
    typename x_enable_if<numeric<T>::is_integer && !numeric<T>::is_signed, bool>::type decode(const char *field, T &val, const Extend *ext) {
        int idx = find(field);
        if (idx >= 0) {
            val = (T)std::strtoul(_row[idx], NULL, 10);
            return true;
        }
        return false;
    }
    // signed integer
    template <class T>
    typename x_enable_if<numeric<T>::is_integer && numeric<T>::is_signed, bool>::type decode(const char *field, T &val, const Extend *ext) {
        int idx = find(field);
        if (idx >= 0) {
            const char *d = _row[idx];
            if (d==NULL || d[0]=='\0') {
                val = 0;
                return true;
            }
            switch (_res->fields[idx].type) {
            case MYSQL_TYPE_TIME: {
                    std::vector<std::string> ss;
                    if (3 == Util::split(ss, d, ':')) {
                        long int s0 = std::strtol(ss[0].c_str(), NULL, 10);
                        long int s1 = std::strtol(ss[1].c_str(), NULL, 10);
                        long int s2 = std::strtol(ss[2].c_str(), NULL, 10);
                        if (s0 >= 0) {
                            val = 1;
                        } else {
                            val = -1;
                            s0 *= -1;
                        }
                        val *= (T)(s0*3600 + s1*60 +s2);
                    }
                }
                break;
            case MYSQL_TYPE_DATETIME:
            case MYSQL_TYPE_TIMESTAMP:{ // parse to unix timestamp, seconds, discard fractional part
                    tm t;
                    strptime(d, "%F %T", &t);
                    val = (T)mktime(&t);
                }
                break;
            default:
                val = (T)std::strtol(_row[idx], NULL, 10);
            }
            return true;
        }
        return false;
    }
    // float
    template <class T>
    typename x_enable_if<numeric<T>::is_float, bool>::type decode(const char *field, T &val, const Extend *ext) {
        int idx = find(field);
        if (idx >= 0) {
            val = (T)std::strtod(_row[idx], NULL);
            return true;
        }
        return false;
    }

    // mysql covert type
    template <class T>
    typename x_enable_if<is_xpack_mysql_xtype<T>::value, bool>::type decode(const char *field, T &val, const Extend *ext) {
        int idx = find(field);
        if (idx >= 0) {
            return xpack_mysql_decode(&_res->fields[idx], _row[idx], val, ext);
        }
        return false;
    }

    // class/struct defined XPACK/XPACK_OUT, default use json to parse, if use other, use xpack_mysql_decode
    template <class T>
    typename x_enable_if<(T::__x_pack_value || is_xpack_out<T>::value) && !is_xpack_mysql_xtype<T>::value, bool>::type decode(const char *field, T &val, const Extend *ext) {
        (void)ext;
        int idx = find(field);
        if (idx >= 0) {
            return xpack::json::decode(_row[idx], val);
        }
        return false;
    }

};


}

#endif
