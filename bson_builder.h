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

#ifndef __X_PACK_BSON_BUILDER_H
#define __X_PACK_BSON_BUILDER_H

#include <stdexcept>
#include "bson_encoder.h"

#ifndef X_PACK_SUPPORT_CXX0X // support c++11 or later
#error "need c++11 or later"
#endif

namespace xpack {

class BsonBuilder {
    enum {
        Unknow = 0,
        ObjectBegin = 1<<0,
        ObjectEnd = 1<<1,
        ArrayBegin = 1<<2,
        ArrayEnd = 1<<3,
        Comma = 1<<4,
        Colon = 1<<5,
        Integer = 1<<6,
        Float = 1<<7,
        String = 1<<8,
        Bool = 1<<9,
        Variable = 1<<10,
        BEof = 1<<11
    };
    
    std::string tokenName(int type) const {
        std::string ret;
        if (0 != (ObjectBegin&type)) {
            ret += "/{";
        }
        if (0 != (ObjectEnd&type)) {
            ret += "/}";
        }
        if (0 != (ArrayBegin&type)) {
            ret += "/[";
        }
        if (0 != (ArrayEnd&type)) {
            ret += "/]";
        }
        if (0 != (Comma&type)) {
            ret += "/,";
        }
        if (0 != (Colon&type)) {
            ret += "/:";
        }
        if (0 != ((Float|Integer|Bool)&type)) {
            ret += "/number/bool";
        }
        if (0 != (String&type)) {
            ret += "/string";
        }
        if (0 != (Variable&type)) {
            ret += "/variable";
        }
        
        if (!ret.empty()) {
            ret = ret.substr(1);
        }
        return ret;
    }
    struct Token {
        int type; // see enum
        union {
            const char *sval;
            long   lval;
            double dval;
            bool   bval;
        };
        Token():type(Unknow),sval(NULL){}
    };
    struct Item {
        Token tk;
        const char *key;
        bool kvar; // key is variable
        Item():tk(),key(NULL), kvar(false) {
        }
    };
    struct Encoder {
        const std::vector<const Item*>* items;
        BsonWriter wr;
        XEncoder<BsonWriter> en;

        size_t idx;  // items index;
        size_t vidx; // variable index;

        std::vector<std::string> vkeys; // store variable key
        Item tmpIt;  // key:value when key is a variable, copy item to here
        Item *pIt;   // point to tmpIt

        std::vector<size_t> arrayIndex;
        std::vector<const char*> objKeys;   // store object key. use in object end
        std::vector<const char*> arrKeys;   // store array key, use in array end

        Encoder(const std::vector<const Item*>&its):items(&its), en(wr), idx(0), vidx(0), pIt(NULL){}

        // array key. index: "0", "1", "2", "3"
        const char *Key(const char *raw) {
            if (NULL != raw) {
                return raw;
            } else if (arrayIndex.size()>0) {
                size_t idx = arrayIndex.size() - 1;
                return wr.IndexKey(arrayIndex[idx]++);
            } else {
                return NULL;
            }
        }
    };

public:
    BsonBuilder(const std::string& _fmt):raw(_fmt),en(NULL) {
        dup = new char[raw.length()+1];
        strcpy(dup, raw.c_str());
        if (parse()) {
            for (size_t i=0; i<items.size(); ++i) {
                if (items[i]->kvar || items[i]->tk.type==Variable) {
                    return;
                }
            }

            // no variable. pre build it
            bstr = Encode();
        }
    }

    ~BsonBuilder() {
        if (NULL != dup) {
            for (size_t i=0; i<items.size(); ++i) {
                delete items[i];
            }
            delete[] dup;
            dup = NULL;
        } else if (NULL != en) {
            delete en;
            en = NULL;
        }
    }

    std::string Error() const {
        return fmtErr;
    }

    template <typename... Args>
    std::string Encode(Args... args) {
        if (!bstr.empty()) {
            return bstr;
        } else if (!fmtErr.empty()) {
            return "";
        }

        BsonBuilder bd(*this);
        bd.iencode(args...);
        bd.end(NULL);
        return bd.en->en.String();
    }

    template <typename... Args>
    std::string EncodeAsJson(Args... args) {
        if (!fmtErr.empty()) {
            return "";
        }

        BsonBuilder bd(*this);
        bd.iencode(args...);
        bd.end(NULL);

        return bd.en->wr.Json();
    }
    template <typename... Args>
    static std::string En(const std::string&fmt, Args... args) {
        BsonBuilder bb(fmt);
        return bb.Encode(args...);
    }

private: // encoder
    BsonBuilder(const BsonBuilder&bd):dup(NULL) {
        en = new Encoder(bd.items);
    }

    // implement iencode()/iencode(T)/iencode(T, Args...) to expand template arguments
    void iencode() {
    }
    void iencode(const char*val) { // xencoder not allow pointer
        std::string s(val);
        encode(s);
        ++en->vidx;
    }
    template <typename T>
    void iencode(const T &val) {
        encode(val);
        ++en->vidx;
    }
    template <typename T, typename... Args>
    void iencode(const T& first, Args... args) {
        iencode(first);
        iencode(args...);
    }

    template <typename T>
    BsonBuilder& encode(const T &val) {
        // encode data items without variable
        for (; en->idx<en->items->size(); ++en->idx) {
            const Item *it = en->items->at(en->idx);
            if ((!it->kvar) && it->tk.type!=Variable) {
                add(it, 1);
            } else {
                break;
            }
        }

        if (en->idx >= en->items->size()) { // finish
            return *this;
        } else if (NULL != en->pIt) {       // key:val. key and val is variable, key replaced in last encode
            add(en->pIt, val);
            en->pIt = NULL;
            ++en->idx;
            return *this;
        }

        const Item *it = en->items->at(en->idx);
        if (it->kvar) {         // key:val and key is variable, we fill string to key
            en->tmpIt = *it;    // copy item info
            en->pIt = &en->tmpIt;
            en->pIt->key = dumpStr(val);    // dump key string
            en->pIt->kvar = false;
            if (it->tk.type != Variable) {  // value not variable
                add(en->pIt, 1);
                en->pIt = NULL;
                ++en->idx;
            }
        } else { // val is variable
            add(it, val);
            ++en->idx;
        }
        return *this;
    }

    // add items to encoder
    template <typename T>
    void add(const Item *it, const T&val) {
        const char *key = en->Key(it->key);

        switch (it->tk.type) {
        case ObjectBegin:
            en->en.ob(key);
            en->objKeys.push_back(key);
            break;
        case ObjectEnd:
            en->en.oe(en->objKeys.back());
            en->objKeys.pop_back();
            break;
        case ArrayBegin:
            en->arrayIndex.push_back(0);
            en->en.ab(key);
            en->arrKeys.push_back(key);
            break;
        case ArrayEnd:
            en->arrayIndex.pop_back();
            en->en.ae(en->arrKeys.back());
            en->arrKeys.pop_back();
            break;
        case Integer:
            en->en.add(key, it->tk.lval);
            break;
        case Float:
            en->en.add(key, it->tk.dval);
            break;
        case String:
            en->en.add(key, std::string(it->tk.sval));
            break;
        case Bool:
            en->en.add(key, it->tk.bval);
            break;
        case Variable:
            en->en.add(key, val);
            break;
        }
    }

    // encode the last none variable item
    void end(std::string *err) {
        for (; en->idx<en->items->size(); ++en->idx) {
            const Item *it = en->items->at(en->idx);
            if (it->kvar || it->tk.type==Variable) {
                if (NULL != err) {
                    *err = "less variable";
                }
                return;
            }
            add(it, 1);
        }
    }

    // type to const char*
    const char *dumpStr(const std::string&str) {
        en->vkeys.push_back(str);
        return en->vkeys[en->vkeys.size()-1].c_str();
    }
    const char *dumpStr(const char *key) {
        return key;
    }

    template <typename T>
    const char *dumpStr(const T&val) {
        (void)val;
        std::string err = "variable "+Util::itoa(en->vidx)+" is a key, must be std::string or const char*";
        throw std::runtime_error(err);
        return "";
    }

    bool isEnd(char ch) {
        return ch==' ' || ch==':'  || ch==',' || ch=='}' || ch==']'|| ch=='\0';
    }

    // a simple bson string parser
    bool parse() {
        std::vector<char> stack;
        Item *item;
        char *p = dup;
        char *end = p;
        int keyIndex = -1;
        char ch;
        bool waitkey = false; // {key:value, expect key

        int exp = ObjectBegin; // expect type
        while (true) {
            item = new Item;
            p = end;
            nextToken(p, &end, item->tk);

            if (item->tk.type == BEof) {
                delete item;
                if (!fmtErr.empty()) {
                    return false;
                }

                if (stack.size() > 0) {
                    if ((ch=stack[stack.size()-1]) == '{') { // in object
                        fmtErr = "miss } at the end of string";
                    } else {
                        fmtErr = "miss ] at the end of string";
                    }
                    return false;
                }
                return true;
            } else if (item->tk.type == Unknow) {
                fmtErr = "unknow token:";
                fmtErr += p;
                delete item;
                break;
            }

            if ((item->tk.type&exp) == 0) {
                fmtErr = "unexpected token. expect["+tokenName(exp)+"]. but get["+tokenName(item->tk.type)+"]";
                if (NULL != item->tk.sval) {
                    fmtErr += ". in [";
                    fmtErr += item->tk.sval;
                    fmtErr += "]";
                }
                delete item;
                break;
            }

            // key:value parse value done
            if (keyIndex >= 0) {
                const Item *it = items[keyIndex];
                items.pop_back();
                keyIndex = -1;
                if (it->tk.type == String) {
                    item->key = it->tk.sval;
                } else {
                    item->kvar = true;
                }
                delete it;
            }

            switch (item->tk.type) {
            case ObjectBegin:
                items.push_back(item);
                exp = ObjectEnd | Variable | String;
                stack.push_back('{');
                waitkey = true;
                break;
            case ObjectEnd:
                waitkey = false;
            case ArrayEnd:
                items.push_back(item);
                stack.pop_back();
                if (stack.size() == 0) {
                    exp = BEof;
                } else if ((ch=stack[stack.size()-1]) == '{') { // in object
                    exp = ObjectEnd | Comma | String | Variable;
                } else {
                    exp = ArrayEnd | Comma | Integer | Float | String | Bool | Variable;
                }
                break;
            case ArrayBegin:
                items.push_back(item);
                exp = ObjectBegin | ArrayBegin | ArrayEnd | Integer | Float | String | Bool | Variable;
                stack.push_back('[');
                break;
            case Comma:
                if ((ch=stack[stack.size()-1]) == '{') { // in object
                    exp = String | Variable;
                    waitkey = true;
                } else {
                    exp = ObjectBegin | ArrayBegin | Integer | Float | String | Bool | Variable;
                }
                delete item;
                break;
            case Colon:
                exp = ObjectBegin | ArrayBegin | Integer | Float | String | Bool | Variable;
                keyIndex = int(items.size()) - 1;
                waitkey = false;
                delete item;
                break;
            case Integer:
            case Float:
            case String:
            case Bool:
            case Variable:
                items.push_back(item);
                if ((ch=stack[stack.size()-1]) == '{') { // in object
                    if (waitkey) {
                        waitkey = false;
                        exp = Colon;
                    } else {
                        exp = ObjectEnd | Comma;
                    }
                } else {
                    exp = ArrayEnd | Comma | Integer | Float | String | Bool | Variable;
                }
                break;
            }
        }
        return false;
    }

    // get next token from data, may modify data
    void nextToken(char* data, char**end, Token &tk) {
        char   ch;
        size_t i = 0;

        *end = NULL;
        while (data[i] == ' ') {
            i++;
        }

        if (data[i] == '\0') {
            tk.type = BEof;
            return;
        }

        tk.sval = data+i;
        while ((ch=data[i++]) != '\0') {
            if (tk.type == String) {
                if (ch != '\'') {
                    continue;
                } else if (isEnd(data[i])) {
                    data[i-1] = '\0';
                    *end = data+i;
                } else {
                    tk.type = Unknow;
                }
                return;
            } else if (tk.type == Integer || tk.type == Float) {
                if (ch>='0' && ch<='9') {
                    continue;
                } else if (ch == '.') {
                    tk.type = Float;
                    continue;
                } else if (isEnd(ch)) {
                    if (tk.type == Integer) {
                        data[i-1] = '\0';
                        if (Util::atoi(tk.sval, tk.lval)) {
                            *end = data+i-1;
                        } else {
                            tk.type = Unknow;
                        }
                        data[i-1] = ch;
                    } else {
                        char *dend;
                        tk.dval = strtod(tk.sval, &dend);
                        if (dend != data+i-1) {
                            tk.type = Unknow;
                        } else {
                            *end = dend;
                        }
                    }
                } else {
                    tk.type = Unknow;
                }
                return;
            } else if (tk.type == Variable) {
                if ((ch>='0' && ch<='9') || (ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || ch=='_') {
                    continue;
                } else if (isEnd(ch)) {
                    *end = data+i-1;
                } else {
                    tk.type = Unknow;
                }
                return;
            } else if (ch == '{') {
                tk.type = ObjectBegin;
                break;
            } else if (ch == '}') {
                tk.type = ObjectEnd;
                break;
            } else if (ch == '[') {
                tk.type = ArrayBegin;
                break;
            } else if (ch == ']') {
                tk.type = ArrayEnd;
                break;
            } else if (ch == ',') {
                tk.type = Comma;
                break;
            } else if (ch == ':') {
                tk.type = Colon;
                break;
            } else if (ch == '\'') { // string
                tk.type = String;
                tk.sval = data+i;
            } else if (ch == '?') {
                tk.type = Variable;
            } else if (ch=='-' || ch=='+' || (ch>='0' && ch<='9')) {
                tk.type = Integer;
                tk.sval = data+i-1;
            } else if (ch == 't') { //
                if (strncmp(data+i-1, "true", 4) == 0 && isEnd(data[i+3])) {
                    tk.type = Bool;
                    tk.bval = true;
                    *end = data+i+3;
                }
                return;
            } else if (ch == 'f') { // 
                if (strncmp(data+i-1, "false", 5) == 0 && isEnd(data[i+4])) {
                    tk.type = Bool;
                    tk.bval = false;
                    *end = data+i+4;
                }
                return;
            } else {
                return;
            }
        }

        if (tk.type != Unknow) {
            if (*end==NULL && (tk.type==String||tk.type==Float||tk.type==Integer||tk.type==Variable)) {
                fmtErr = "no end for string/integer/float/variable";
                tk.type = BEof;
            } else {
                *end = data+i;
            }
        }
    }

    std::string raw;
    std::string fmtErr;
    std::string lastErr;
    std::string bstr;   // for that without any variable
    char* dup;          // for parse
    std::vector<const Item*> items;
    Encoder *en;
};

}

#endif
