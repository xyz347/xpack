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

#ifndef __X_PACK_UTIL_H
#define __X_PACK_UTIL_H

#include <string>
#include <vector>

#include <stdio.h>
#include <string.h>

#include "traits.h"
#include "numeric.h"

namespace xpack {

struct cmp_str {
   bool operator()(char const *a, char const *b) const {
      return strcmp(a, b) < 0;
   }
};

class Util {
public:
    static size_t split(std::vector<std::string>&slice, const std::string&str, char c) {
        size_t last = 0;
        size_t pos = 0;
        while (std::string::npos != (pos=str.find(c, last))) {
            slice.push_back(str.substr(last, pos-last));
            last = pos+1;
        }

        slice.push_back(str.substr(last));
        return slice.size();
    }

    static size_t split(std::vector<std::string>&slice, const std::string&str, const std::string& sp) {
        size_t last = 0;
        size_t pos = 0;
        size_t len = sp.length();
        if (len == 0) {
            slice.push_back(str);
            return 1;
        }
        while (std::string::npos != (pos=str.find(sp, last))) {
            slice.push_back(str.substr(last, pos-last));
            last = pos+len;
        }

        slice.push_back(str.substr(last));
        return slice.size();
    }

    // not support float.
    template <class T>
    static typename x_enable_if<numeric<T>::is_integer, std::string>::type itoa(const T&val) {
        char buf[128];
        size_t i = sizeof(buf)-1;

        if (val == 0) {
            return std::string("0");
        }

        T _tmp = val>0?val:-val;
        while (_tmp > 0) {
            buf[i--] = char(int('0')+int(_tmp%10));
            _tmp /= 10;
        }
        if (val < 0) {
            buf[i--] = '-';
        }
        return std::string(&buf[i+1], sizeof(buf)-i-1);
    }

    // not support float. and only decimal
    template <class T>
    static typename x_enable_if<numeric<T>::is_integer, bool>::type atoi(const std::string&s, T&val) {
        if (s.empty()) {
            return false;
        }

        T _tmp = 0;
        size_t i = 0;
        if (s[0] == '-') {
            if (s.length() == 1) {
                return false;
            } else if (s.length()>2 && s[1]=='0') {
                return false;
            }

            for (i=1; i<s.length(); ++i) {
                if (s[i]>='0' && s[i]<='9') {
                    T _c = _tmp*10 - (s[i]-'0');
                    if (_c < _tmp) {
                        _tmp = _c;
                    } else if (i > 0) {
                        return false; // overflow
                    }
                } else {
                    return false;
                }
            }
        } else {
            if (s[0] == '+') {
                ++i;
            }
            if (s[i]=='0' && i+1<s.length()) {
                return false;
            }
            for (; i<s.length(); ++i) {
                if (s[i]>='0' && s[i]<='9') {
                    T _c = _tmp*10 + (s[i]-'0');
                    if (_c > _tmp) {
                        _tmp = _c;
                    } else if (i > 0) {
                        return false; // overflow
                    }
                } else {
                    return false;
                }
            }
        }

        val = _tmp;
        return true;
    }
};

}

#endif
