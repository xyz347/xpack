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

#ifndef __X_PACK_XML_ENCODER_H
#define __X_PACK_XML_ENCODER_H

#include <list>
#include <sstream>
#include "xencoder.h"


namespace xpack {

class XmlWriter {
    struct Attr {
        const char* key;
        std::string val;
        Attr(const char*_key, const std::string&_val):key(_key), val(_val){}
    };
    struct Node {
        Node(const char*_key=NULL, const Extend *ext=NULL) {
            (void)ext;
            if (_key != NULL) {
                key = _key;
            }
        }
        ~Node() {
            std::list<Node*>::iterator it;
            for (it = childs.begin(); it!=childs.end(); ++it) {
                delete *it;
            }
        }

        std::string key;        // key for this node
        std::string val;        // value for this node, only for leaf node
        std::list<Attr>  attrs; // attribute
        std::list<Node*> childs;// child nodes

        std::string vec_key;    // key for vector
    };

    friend class XEncoder<XmlWriter>;
    friend class XmlEncoder;
    const static bool support_null = false;
public:
    XmlWriter(int indentCount = -1, char indentChar = ' ', int decimalPlaces = 324):_indentCount(indentCount),_indentChar(indentChar),_decimalPlaces(decimalPlaces) {
        if (_indentCount > 0) {
            if (_indentChar!=' ' && _indentChar!='\t') {
                throw std::runtime_error("indentChar must be space or tab");
            }
        }
        _cur = &_root;
    }
private:
    std::string String() {
        merge();
        return _output;
    }
    inline static const char *Name() {
        return "xml";
    }
    inline const char *IndexKey(size_t index) {
        (void)index;
        return _cur->vec_key.c_str();
    }
    void ArrayBegin(const char *key, const Extend *ext) {
        Node *n = new Node(key, ext);

        if (NULL!=_cur && !_cur->vec_key.empty()) { // vector<vector<...>>
            n->vec_key = n->key;
        } else if (NULL != ext && NULL != ext->alias) {
            if (!ext->alias->Flag("xml", "vl", &n->vec_key)) {  // forward compatible, support vector label
                if (ext->alias->Flag("xml", "sbs")) {           // no top label, vector item will side by side
                    n->vec_key.swap(n->key); // n->vec_key=key and n->key="";
                } else {
                    n->vec_key = n->key;     // has top level
                }
            }
        } else {
            n->vec_key = n->key;
        }

        _cur->childs.push_back(n);

        _stack.push_back(_cur);
        _cur = n;
    }
    void ArrayEnd(const char *key, const Extend *ext) {
        (void)key;
        (void)ext;
        _cur = _stack.back();
        _stack.pop_back();
    }
    void ObjectBegin(const char *key, const Extend *ext) {
        Node *n = new Node(key, ext);
        _cur->childs.push_back(n);

        _stack.push_back(_cur);
        _cur = n;
    }
    void ObjectEnd(const char *key, const Extend *ext) {
        (void)key;
        (void)ext;
        _cur = _stack.back();
        _stack.pop_back();
    }
    bool WriteNull(const char*key, const Extend *ext) {
        static std::string empty;
        return this->encode_string(key, empty, ext);
    }
    // string
    bool encode_string(const char*key, const std::string &val, const Extend *ext) {
        if (Extend::Attribute(ext)) {
            _cur->attrs.push_back(Attr(key, string_quote(val)));
        } else if (Extend::XmlContent(ext)) {
            _cur->val = val;
        } else if (!Extend::AliasFlag(ext, "xml", "cdata")) {
            Node *n = new Node(key);
            n->val = string_quote(val);
            _cur->childs.push_back(n);
        } else {
            Node *n = new Node(key);
            n->val = "<![CDATA[" + val + "]]>";
            _cur->childs.push_back(n);
        }
        return true;
    }
    // bool
    bool encode_bool(const char*key, const bool &val, const Extend *ext) {
        std::string bval;
        if (val) {
            bval = "true";
        } else {
            bval = "false";
        }

        if (Extend::Attribute(ext)) {
            _cur->attrs.push_back(Attr(key, bval));
        } else if (Extend::XmlContent(ext)) {
            _cur->val = bval;
        } else {
            Node *n = new Node(key);
            n->val = bval;
            _cur->childs.push_back(n);
        }
        return true;
    }
    // integer
    template <class T>
    typename x_enable_if<numeric<T>::is_integer, bool>::type encode_number(const char*key, const T& val, const Extend *ext) {
        if (val==0 && Extend::OmitEmpty(ext)) {
            return false;
        } else if (Extend::Attribute(ext)) {
            _cur->attrs.push_back(Attr(key, Util::itoa(val)));
        } else if (Extend::XmlContent(ext)) {
            _cur->val = Util::itoa(val);
        } else {
            Node *n = new Node(key);
            n->val = Util::itoa(val);
            _cur->childs.push_back(n);
        }
        return true;
    }

    // float
    template <class T>
    typename x_enable_if<numeric<T>::is_float, bool>::type encode_number(const char*key, const T &val, const Extend *ext) {
        if (val==0 && Extend::OmitEmpty(ext)) {
            return false;
        } else {
            std::ostringstream os;
            os.precision(_decimalPlaces+1);
            os<<val;
            std::string fval = os.str();

            if (Extend::Attribute(ext)) {
                _cur->attrs.push_back(Attr(key, fval));
            } else if (Extend::XmlContent(ext)) {
                _cur->val = fval;
            } else {
                Node *n = new Node(key);
                n->val = fval;
                _cur->childs.push_back(n);
            }
        }
        return true;
    }

    // escape string
    static std::string string_quote(const std::string &val) {
        std::string ret;
        ret.reserve(val.length()*2);

        for (size_t i=0; i<val.length(); ++i) {
            int c = (int)(val[i]) & 0xFF;
            switch (c) {
              case '<':
                ret += "&lt;";
                break;
              case '>':
                ret += "&gt;";
                break;
              case '&':
                ret += "&amp;";
                break;
              case '\'':
                ret += "&apos;";
                break;
              case '\"':
                ret += "&quot;";
                break;
              default:
                if(c >= ' ') {
                    ret.push_back(val[i]);
                } else {
                    ret += "\\x";
                    unsigned char uch = (unsigned char)val[i];
                    unsigned char h = (uch>>4)&0xf;
                    unsigned char l = uch&0xf;
                    if (h < 10) {
                        ret.push_back(h+'0');
                    } else {
                        ret.push_back(h-10+'A');
                    }
                    if (l < 10) {
                        ret.push_back(l+'0');
                    } else {
                        ret.push_back(l-10+'A');
                    }
                }
            }
        }

        return ret;
    }



    void appendNode(const Node *nd, int depth) {
        bool indentEnd = true;

        if (!nd->key.empty()) { // for array that without label
            indent(depth);
            _output.push_back('<');
            _output += nd->key;
        } else if (depth > 0) {
            depth--;
        }

        // output attribute
        std::list<Attr>::const_iterator it;
        for (it=nd->attrs.begin(); it!=nd->attrs.end(); ++it) {
            _output.push_back(' ');
            _output += it->key;
            _output += "=\"";
            _output += it->val;
            _output.push_back('"');
        }

        // no content, end object
        if (nd->val.empty() && nd->childs.size()==0) {
            if (!nd->key.empty()) {
                _output += "/>";
            }
            return;
        }

        // key finished
        if (!nd->key.empty()) {
            _output.push_back('>');
        }
        if (!nd->val.empty()) {
            _output += nd->val;
            indentEnd = false;
        } else {
            std::list<Node*>::const_iterator it;
            for (it=nd->childs.begin(); it!=nd->childs.end(); ++it) {
                appendNode(*it, depth+1);
            }
        }

        if (!nd->key.empty()) {
            if (indentEnd) {
                indent(depth);
            }
            _output += "</";
            _output += nd->key;
            _output.push_back('>');
        }
    }
    void merge() {
        if (_output.length() > 0 || _root.childs.size()==0) {
            return;
        }
        
        appendNode(_root.childs.front(), 0);
    }

    void indent(int depth) {
        if (_indentCount < 0) {
            return;
        }
        _output.push_back('\n');
        if (_indentCount == 0) {
            return;
        }
        for (int i=0; i<depth*_indentCount; ++i) {
            _output.push_back(_indentChar);
        }
    }

    Node _root;
    Node *_cur;
    std::list<Node*> _stack;

    std::string _output;

    int  _indentCount;
    char _indentChar;

    int _decimalPlaces;
};


class XmlEncoder {
public:
    XmlEncoder() {
        indentCount = -1;
        indentChar = ' ';
        maxDecimalPlaces = 324;
    }
    XmlEncoder(int _indentCount, char _indentChar, int _maxDecimalPlaces = 324) { // compat
        indentCount = _indentCount;
        indentChar = _indentChar;
        maxDecimalPlaces = _maxDecimalPlaces;
    }

    void SetMaxDecimalPlaces(int _maxDecimalPlaces) {
        maxDecimalPlaces = _maxDecimalPlaces;
    }

    template <class T>
    std::string encode(const T&val, const std::string&root) {
        XmlWriter wr(indentCount, indentChar, maxDecimalPlaces);
        XEncoder<XmlWriter> en(wr);
        en.encode(root.c_str(), val, NULL);
        return wr.String();
    }

private:
    int indentCount;
    char indentChar;
    int maxDecimalPlaces;
};

}

#endif
