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


#ifndef __X_PACK_SQLITE_DECODER_H
#define __X_PACK_SQLITE_DECODER_H

#include <stdexcept>
#include <vector>
#include <map>
#include <cstdlib>

#include <sqlite.h>

#include "extend.h"
#include "traits.h"
#include "json.h"

#include "string.h"
#include "time.h"

namespace xpack {

class SQLiteDecoder {
    friend class sqlite;
public:
    template <typename T>
    bool decode(const char*key, T&val, const Extend *ext) {
        int idx = find(key);
        if (idx >= 0) {
            return this->decode_type(idx, val, ext);
        }
        return false;
    }
    const char *Name() const { // mysql and sqlite use db
        return "db";
    }

private:
    const char **_res;
    const int _rows;
    const int _cols;
    int _offset;
    std::map<const char*, int, cmp_str> _index;

    SQLiteDecoder(const char**result, int rows, int cols):_res(result), _rows(rows), _cols(cols) {
        for (int i=0; i<cols; ++i) {
            _index[_res[i]] = i;
        }
        _offset = cols;
    }

    // decode to struct
    template <class T>
    inline XPACK_IS_XPACK(T) decode_top(T& val, const Extend *ext) {
        if (_rows > 0) {
            val.__x_pack_decode(*this, val, ext);
            return true;
        }
        return false;
    }
    template <class T>
    inline XPACK_IS_XPACK(T) decode_top(std::vector<T>& val, const Extend *ext) {
        for (int i=0; i<_rows; ++i) {
            val.push_back(T());
            T& tmp = val.back();
            tmp.__x_pack_decode(*this, tmp, ext);
            _offset += _cols;
        }
        return true;
    }
    template <class T>
    inline XPACK_IS_XOUT(T) decode_top(T& val, const Extend *ext) {
        if (_rows > 0) {
            __x_pack_decode_out(*this, val, ext);
        }
        return true;
    }
    template <class T>
    inline XPACK_IS_XOUT(T) decode_top(std::vector<T>& val, const Extend *ext) {
        for (int i=0; i<_rows; ++i) {
            val.push_back(T());
            __x_pack_decode_out(*this, val.back(), ext);
            _offset += _cols;
        }
        return true;
    }

    // decode special column of first row
    template <class T>
    bool decode_column(const char*field, T&val, const Extend *ext) {
        int idx;
        if (_rows>0 && (0 <= (idx = find(field)))) {
            return decode_type(idx, val, ext);
        }
        return false;
    }
    // decode special column of rows
    template <class T>
    bool decode_column(const char*field, std::vector<T>&val, const Extend *ext) {
        int idx;
        if (0 > (idx = find(field))) {
            return false;
        }
        for (int i=0; i<_rows; ++i) {
            T t;
            decode_type(idx, t, ext);
            val.push_back(t);
            _offset += _cols;
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

    // std::string
    bool decode_type(const int idx, std::string &val, const Extend *ext) {
        const char* s;
        if (NULL != (s = _res[idx + _offset])) {
            val = s;
        }
        return true;
    }
    // bool
    bool decode_type(const int idx, bool &val, const Extend *ext) {
        int tmp;
        if (decode_type(idx, tmp, ext)) {
            val = tmp != 0;
            return true;
        }
        return false;
    }
    // integer
    template <class T>
    typename x_enable_if<numeric<T>::is_integer, bool>::type decode_type(const int idx, T &val, const Extend *ext) {
        const char* s;
        if (NULL != (s = _res[idx + _offset])) {
            val = (T)std::strtoul(s, NULL, 10);
        } // else val = 0 ???
        return true;
    }
    // float
    template <class T>
    typename x_enable_if<numeric<T>::is_float, bool>::type decode_type(const int idx, T &val, const Extend *ext) {
        const char* s;
        if (NULL != (s = _res[idx + _offset])) {
            val = (T)std::strtod(s, NULL);
        }
        return true;
    }
    // class/struct defined XPACK/XPACK_OUT, default use json to parse
    template <class T>
    inline XPACK_IS_XOUT(T) decode_type(const int idx, T &val, const Extend *ext) {
        (void)ext;
        const char* s;
        if (NULL != (s = _res[idx + _offset])) {
            return xpack::json::decode(s, val);
        }
        return true;
    }
    template <class T>
    inline XPACK_IS_XPACK(T) decode_type(const int idx, T &val, const Extend *ext) {
        (void)ext;
        const char* s;
        if (NULL != (s = _res[idx + _offset])) {
            return xpack::json::decode(s, val);
        }
        return true;
    }
};


}

#endif
