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

#ifndef __X_PACK_BSON_TYPE_H
#define __X_PACK_BSON_TYPE_H

#include <map>
#include <string>
#include <sstream>
#include <ctime>
#include <time.h>
#include <stdint.h>

#include <libbson-1.0/bson.h>

#include "xencoder.h"
#include "xdecoder.h"
#include "xpack.h"

/*
// define in this file
bson_date_time_t
bson_timestamp_t
bson_binary_t
bson_regex_t

// define in bson.h
bson_oid_t
bson_decimal128_t
*/

// define in global namespace for consistency with bson
struct bson_timestamp_t {
    uint32_t timestamp;
    uint32_t increment;
    bson_timestamp_t(const uint32_t&t=0, const uint32_t&i=0):timestamp(t),increment(i){}
    XPACK(A(timestamp, "t", increment, "i"));
};
struct bson_regex_t {
    std::string pattern; // libbson not escape string, this should be a bug
    std::string options;
    // ver==1 {"$regularExpression": {pattern: <string>, "options": <string>"}}
    // ver==2 {"$regex: <string>, $options: <string>"}
    int ver; // ver==1 
    bson_regex_t():ver(2){}
};
struct bson_date_time_t {
    int64_t ts; // milliseconds
    bson_date_time_t(const int64_t&t=0):ts(t){}
    operator int64_t() const {return ts;}

    // parse rfc3339 time format(milliseconds). C++ doesn't even have a full-featured standard time library T_T
    // "2000-01-01T01:01:01[.001]Z[07:00]" string should ends with \0
    bool parse_rfc3339(const char buf[]) {
        struct tm tm;
        memset((void*)&tm, 0, sizeof(tm));

        const char *end = strptime(buf, "%Y-%m-%dT%H:%M:%S", &tm);
        if (end == NULL) {
            return false;
        }

        int msec = 0;
        int offset = 0;
        try {
            size_t pos;
            if (*end == '.') {
                msec = std::stoi(end+1, &pos);
                if (pos==0 || pos > 3) {
                    return false;
                }
                end += pos+1;
            }
            if (*end != 'Z') {
                pos = 0;
                int hour = std::stoi(end, &pos);
                if (pos == 0 || pos > 3) {
                    return false;
                }
                end += pos;
                if (*end != ':') {
                    return false;
                }
                int min = std::stoi(end+1, &pos);
                if (hour > 0) {
                    offset = hour*3600 + min*60;
                } else {
                    offset = -(-hour*3600 + min*60);
                }
            } else {
                ++end;
                if (*end != '\0') {
                    return false;
                }
            }
        } catch(...) {
            return false;
        }

        time_t tmp = mktime(&tm);
        ts = tmp*1000 + msec - offset - timezone*1000;
        return true;
    }
};
struct bson_binary_t {
    std::string data;        // std::string is more general, so don't use std::basic_string<uint8_t>
    bson_subtype_t subType;

    inline static const char *b64() {
        return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    }

    // b64_ntop and b64_pton maybe should move to a public place
    static void b64_ntop(const uint8_t *bin, size_t size, std::string&out) {
        const char *b = b64();

        out.reserve((size+2)/3*4);
        size_t n = size/3*3;
        size_t i;
        for (i=0; i<n; i+=3) {
            out.push_back(b[bin[i]>>2]);
            out.push_back(b[((bin[i]&0x3)<<4) | (bin[i+1]>>4)]);
            out.push_back(b[((bin[i+1]&0xf)<<2) | (bin[i+2]>>6)]);
            out.push_back(b[bin[i+2]&0x3f]);
        }
        size_t left = size%3;
        if (left == 1) {
            out.push_back(b[bin[i]>>2]);
            out.push_back(b[(bin[i]&0x3)<<4]);
            out.push_back('=');
            out.push_back('=');
        } else if (left == 2) {
            out.push_back(b[bin[i]>>2]);
            out.push_back(b[((bin[i]&0x3)<<4) | (bin[i+1]>>4)]);
            out.push_back(b[(bin[i+1]&0xf)<<2]);
            out.push_back('=');
        }
    }
    static bool b64_pton(const std::string&in, std::string&out) {
        static uint8_t bmap[256] = {0};
        if (bmap[255] == 0) { // init bmap, thread safe
            for (size_t i=0; i<255; i++) bmap[i] = 0xff;
            const char *p = b64();
            for (uint8_t i=0; p[i]!='\0'; ++i) bmap[(int)p[i]] = i;
            bmap[255] = 0xff;
        }

        size_t size = in.length();
        if (size==0 || 0 != (size&0x3)) {
            return false;
        }
        out.reserve((size+3)/4*3);

        const char *data = in.data();
        for (size_t i=0; i<size; i+=4) {
            uint8_t i1 = bmap[(uint8_t)data[i]];
            uint8_t i2 = bmap[(uint8_t)data[i+1]];
            uint8_t i3 = bmap[(uint8_t)data[i+2]];
            uint8_t i4 = bmap[(uint8_t)data[i+3]];
            if (i1!=0xff && i2!=0xff) {
                out.push_back((char)((i1<<2) | (i2>>4)));
            } else {
                return false;
            }

            if (i3 != 0xff) {
                out.push_back((char)((i2<<4) | (i3>>2)));
            } else if (data[i+2]=='=' && data[i+3]=='=' && i+4==size) {
                return true;
            } else {
                return false;
            }
            if (i4 != 0xff) {
                out.push_back((char)((i3<<6) | i4));
            } else if (data[i+3]=='=' && i+4==size) {
                return true;
            } else {
                return false;
            }
        }
        return true;
    }
};

namespace xpack {

// bson type to json
// ref https://github.com/mongodb/specifications/blob/master/source/extended-json.rst#conversion-table
// Relaxed Extended JSON Format Only

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ObjectId ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
// {"$oid":"xxxxx"}
template<>
struct is_xpack_xtype<bson_oid_t> {static bool const value = true;};
template<class Decoder>
bool xpack_xtype_decode(Decoder &de, bson_oid_t &val, const Extend *ext) {
    (void)ext;
    char buf[25];
    bool ret = de.decode("$oid", buf, NULL);
    if (ret) {
        bson_oid_init_from_string(&val, buf);
    }
    return ret;
}
template<class Encoder>
bool xpack_xtype_encode(Encoder &en, const char*key, const bson_oid_t &val, const Extend *ext) {
    char buf[25];
    bson_oid_to_string(&val, buf);
    en.ObjectBegin(key, ext);
    en.encode("$oid", buf, NULL);
    en.ObjectEnd(key, ext);
    return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ date time ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// {"$date": 12345} 
// {"$date":"2006-01-02T15:04:05Z"}
// {"$date":{"$numberLong":"123456"}}
template<>
struct is_xpack_xtype<bson_date_time_t> {static bool const value = true;};
template<class Decoder>
bool xpack_xtype_decode(Decoder &de, bson_date_time_t &val, const Extend *ext) {
    (void)ext;
    static const char *dkey = "$date";

    bool ret;
    try {
        ret = de.decode(dkey, val.ts, NULL);
        return ret;
    } catch (...){}

    char buf[25];
    try {
        ret = de.decode(dkey, buf, NULL);
        if (ret) {
            ret = val.parse_rfc3339(buf);
        }
        return ret;
    } catch (...) {}

    try {
        ret = de.decode("$numberLong", buf, NULL);
        if (ret) {
            ret = Util::atoi(buf, val.ts);
        }
        return ret;
    } catch (...) {}

    return false;
}
template<class Encoder>
bool xpack_xtype_encode(Encoder &en, const char*key, const bson_date_time_t &val, const Extend *ext) {
    en.ObjectBegin(key, ext);
    if (val.ts >= 0) {
        char buf[32];
        int msecs = int(val.ts % 1000);
        time_t secs = (time_t)(val.ts/1000);

        #ifndef _MSC_VER
        struct tm posix_date;
        size_t sz = strftime(buf, sizeof buf, "%FT%T", gmtime_r(&secs, &posix_date));
        #else
        size_t sz = strftime(buf, sizeof buf, "%FT%T", gmtime(&secs));
        #endif

        if (msecs > 0) {
            sprintf(&buf[sz], ".%03dZ", msecs);
        } else {
            strcpy(&buf[sz], "Z");
        }
        en.encode("$date", buf, NULL);
    } else { // key:{"$date":{"$numberLong":ts}} ts use string not int 
        en.ObjectBegin("$date", NULL);
        en.encode("$numberLong", Util::itoa(val.ts), NULL);
        en.ObjectEnd("$date", NULL);
    }
    en.ObjectEnd(key, ext);
    return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ timestamp ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
// {"$timestamp":{"t":xxx, "i":xxx}}
template<>
struct is_xpack_xtype<bson_timestamp_t> {static bool const value = true;};
template<class Decoder>
bool xpack_xtype_decode(Decoder &de, bson_timestamp_t &val, const Extend *ext) {
    (void)ext;
    bool ret = de.decode_struct("$timestamp", val, NULL);
    return ret;
}
template<class Encoder>
bool xpack_xtype_encode(Encoder &en, const char*key, const bson_timestamp_t &val, const Extend *ext) {
    en.ObjectBegin(key, ext);
    en.encode_struct("$timestamp", val, NULL);
    en.ObjectEnd(key, ext);
    return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ numberDecimal ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
// {"$numberDecimal":"123.4"}
template<>
struct is_xpack_xtype<bson_decimal128_t> {static bool const value = true;};
template<class Decoder>
bool xpack_xtype_decode(Decoder &de, bson_decimal128_t &val, const Extend *ext) {
    (void)ext;
    char buf[BSON_DECIMAL128_STRING];
    bool ret = de.decode("$numberDecimal", buf, NULL);
    if (ret) {
        bson_decimal128_from_string(buf, &val);
    }
    return ret;
}
template<class Encoder>
bool xpack_xtype_encode(Encoder &en, const char*key, const bson_decimal128_t &val, const Extend *ext) {
    char buf[BSON_DECIMAL128_STRING];
    bson_decimal128_to_string(&val, buf);
    en.ObjectBegin(key, ext);
    en.encode("$numberDecimal", buf, NULL);
    en.ObjectEnd(key, ext);
    return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ binary ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
// {"$binary":{"base64":"aGVsbG8=","subType":"80"}}
// {"$binary":"aGVsbG8=", "$type":"80"}
template<>
struct is_xpack_xtype<bson_binary_t> {static bool const value = true;};
template<class Decoder>
bool xpack_xtype_decode(Decoder &de, bson_binary_t &val, const Extend *ext) {
    (void)ext;
    try {
        std::string subType;
        std::string data;
        bool ret1;
        bool ret2;

        Decoder st = de.Find("$type", NULL);
        if (!st) {
            Decoder bd = de.Find("$binary", NULL);
            if (!bd) {
                return false;
            }
            ret1 = bd.decode("subType", subType, NULL);
            ret2 = bd.decode("base64", data, NULL);
        } else {
            ret1 = st.decode(subType, NULL);
            ret2 = de.decode("$binary", data, NULL);
        }

        if (ret1 && (subType.length()==1 || subType.length()==2)) {
            int st = 0;
            for (size_t i=0; i<subType.length(); ++i) {
                int tmp = (int)(subType[i]);
                if (tmp>='0' && tmp<='9') {
                    tmp -= '0';
                } else if (tmp>='a' && tmp<='f') {
                    tmp = 10 + tmp-'a';
                } else if (tmp>='A' && tmp<='F') {
                    tmp = 10 + tmp-'A';
                } else {
                    return false;
                }
                st = (st<<4) + tmp;
            }
            val.subType = (bson_subtype_t)st;
        } else {
            return false;
        }

        if (ret2) {
            ret2 = bson_binary_t::b64_pton(data, val.data);
        }
        return ret2;
    } catch (...){
        return false;
    }

    return true;
}
template<class Encoder>
bool xpack_xtype_encode(Encoder &en, const char*key, const bson_binary_t &val, const Extend *ext) {
    en.ObjectBegin(key, ext);
    en.ObjectBegin("$binary", NULL);

    std::string b64;
    bson_binary_t::b64_ntop((const uint8_t*)val.data.data(), val.data.length(), b64);
    en.encode("base64", b64, NULL);

    std::stringstream ss;
    ss<<std::hex<<val.subType;
    en.encode("subType", ss.str(), NULL);

    en.ObjectEnd("$binary", NULL);
    en.ObjectEnd(key, ext);
    return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ regex ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
template<>
struct is_xpack_xtype<bson_regex_t> {static bool const value = true;};
template<class Decoder>
bool xpack_xtype_decode(Decoder &de, bson_regex_t &val, const Extend *ext) {
    (void)ext;
    Decoder v1 = de.Find("$regularExpression", NULL);
    try {
        if (v1) {
            v1.decode("pattern", val.pattern, NULL);
            v1.decode("options", val.options, NULL);
            val.ver = 1;
        } else {
            de.decode("$regex", val.pattern, NULL);
            de.decode("$options", val.options, NULL);
            val.ver = 2;
        }
    } catch(...){}
    return true;
}
template<class Encoder>
bool xpack_xtype_encode(Encoder &en, const char*key, const bson_regex_t &val, const Extend *ext) {
    en.ObjectBegin(key, ext);
    if (val.ver == 1) {
        en.ObjectBegin("$regularExpression", NULL);
        en.encode("pattern", val.pattern, NULL);
        en.encode("options", val.options, NULL);
        en.ObjectEnd("$regularExpression", NULL);
    } else {
        en.encode("$regex", val.pattern, NULL);
        en.encode("$options", val.options, NULL);
    }
    en.ObjectEnd(key, ext);
    return true;
}


}

#endif
