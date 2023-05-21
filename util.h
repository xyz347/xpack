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
#include <fstream>
#include <stdexcept>
#include <memory>

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
    static bool readfile(const std::string&fname, std::string&data) {
        std::ifstream fs(fname.c_str(), std::ifstream::binary);
        if (!fs) {
            std::string err = "Open file["+fname+"] fail.";
            throw std::runtime_error(err);
        }
        std::string _tmp((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
        data.swap(_tmp);
        return true;
    }
    // if n<0 will split all
    static size_t split(std::vector<std::string>&slice, const std::string&str, char c, int n = -1) {
        size_t last = 0;
        size_t pos = 0;
        int cnt = 0;
        while (std::string::npos != (pos=str.find(c, last))) {
            slice.push_back(str.substr(last, pos-last));
            last = pos+1;
            ++cnt;
            if (n>0 && n>=cnt) {
                break;
            }
        }

        slice.push_back(str.substr(last));
        return slice.size();
    }

    static size_t split(std::vector<std::string>&slice, const std::string&str, const std::string& sp, int n =-1) {
        size_t last = 0;
        size_t pos = 0;
        size_t len = sp.length();
        if (len == 0) {
            slice.push_back(str);
            return 1;
        }
        int cnt = 0;
        while (std::string::npos != (pos=str.find(sp, last))) {
            slice.push_back(str.substr(last, pos-last));
            last = pos+len;
            ++cnt;
            if (n>0 && n>=cnt) {
                break;
            }
        }

        slice.push_back(str.substr(last));
        return slice.size();
    }

    // not support float.
    template <class T>
    static typename x_enable_if<numeric<T>::is_integer, std::string>::type itoa(const T&val) {
        #ifdef X_PACK_SUPPORT_CXX0X
        return std::to_string(val);
        #else
        char buf[128];
        size_t i = sizeof(buf)-1;

        if (val == 0) {
            return std::string("0");
        }

        T _tmp = val>=0?val:val*-1;
        while (_tmp > 0) {
            buf[i--] = char(int('0')+int(_tmp%10));
            _tmp /= 10;
        }
        if (val < 0) {
            buf[i--] = '-';
        }
        return std::string(&buf[i+1], sizeof(buf)-i-1);
        #endif
    }

    #ifdef X_PACK_SUPPORT_CXX0X
    template <class E>
    inline static typename x_enable_if<std::is_enum<E>::value, std::string>::type itoa(const E& val) {
        return itoa((typename std::underlying_type<E>::type)val);
    }
    template <class E>
    static typename x_enable_if<std::is_enum<E>::value, bool>::type atoi(const std::string& key, E& val) {
        typename std::underlying_type<E>::type tmp;
        bool ret = atoi(key, tmp);
        if (ret) {
            val = (E)tmp;
        }
        return ret;
    }
    #endif

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
    template <class T>
    static typename x_enable_if<numeric<T>::is_integer, bool>::type atoi(const char*s, T&val) {
        if (NULL == s) {
            return false;
        }
        std::string _t(s);
        return atoi(_t, val);
    }
};


#if defined(X_PACK_SUPPORT_CXX0X)
    template <class T>
    using x_shared_ptr = std::shared_ptr<T>;
#elif defined(_GNU_SOURCE)
// A very crude implementation of shared_ptr
template <class T>
class x_shared_ptr {
public:
    x_shared_ptr(T *p = NULL):ptr(p) {
        if (NULL != p) {
            counter = new int;
            *counter = 1;
        } else {
            counter = NULL;
        }
    }
    x_shared_ptr(const x_shared_ptr &src) {
        ptr = src.ptr;
        counter = src.counter;
        add_ref(1);
    }
    ~x_shared_ptr() {
        add_ref(-1);
    }
    x_shared_ptr& operator = (const x_shared_ptr &src) {
        add_ref(-1);
        ptr = src.ptr;
        counter = src.counter;
        add_ref(1);
        return *this;
    }
    void reset(T *p = NULL) {
        *this = x_shared_ptr(p);
    }
    T* operator ->() {
        return ptr;
    }
    T* get() {
        return ptr;
    }
    T& operator *() {
        return *ptr;
    }
private:
    void add_ref(int cnt) {
        if (counter == NULL) {
            return;
        }
        int ncnt = __sync_add_and_fetch(counter, cnt);
        if (0 == ncnt) {
            delete counter;
            delete ptr;
        }
    }
    T   *ptr;
    mutable int *counter;
};
#endif

}

#endif
