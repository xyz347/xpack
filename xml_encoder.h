/*
* Copyright (C) 2021 Duowan Inc. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
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

class XmlEncoder:public XEncoder<XmlEncoder> {
private:
    struct Attr {
        const char* key;
        std::string val;
        Attr(const char*_key, const std::string&_val):key(_key), val(_val){}
    };
    struct Node {
        Node(const char*_key=NULL) {
            if (_key != NULL && strlen(_key)>0) {
                key = _key;
            } else {
                key = "x"; // for vector
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
    };

public:
    friend class XEncoder<XmlEncoder>;
    using xdoc_type::encode;

    XmlEncoder(int indentCount=-1, char indentChar=' '):_indentCount(indentCount),_indentChar(indentChar) {
        if (_indentCount > 0) {
            if (_indentChar!=' ' && _indentChar!='\t') {
                throw std::runtime_error("indentChar must be space or tab");
            }
        }
        _cur = &_root;
    }
    ~XmlEncoder() {
    }

    std::string String() {
        merge();
        return _output;
    }
public:
    inline const char *Type() const {
        return "xml";
    }
    inline const char *IndexKey(size_t index) {
        (void)index;
        return "x";
    }
    void ArrayBegin(const char *key) {
        Node *n = new Node(key);
        _cur->childs.push_back(n);

        _stack.push_back(_cur);
        _cur = n;
    }
    void ArrayEnd(const char *key) {
        _cur = _stack.back();
        _stack.pop_back();
    }
    void ObjectBegin(const char *key) {
        Node *n = new Node(key);
        _cur->childs.push_back(n);

        _stack.push_back(_cur);
        _cur = n;
    }
    void ObjectEnd(const char *key) {
        _cur = _stack.back();
        _stack.pop_back();
    }

    // string
    bool encode(const char*key, const std::string &val, const Extend *ext) {
        if (val.empty() && Extend::OmitEmpty(ext)) {
            return false;
        } else if (Extend::Attribute(ext)) {
            _cur->attrs.push_back(Attr(key, StringQuote(val)));
        } else {
            Node *n = new Node(key);
            n->val = StringQuote(val);
            _cur->childs.push_back(n);
        }
        return true;
    }

    // bool
    bool encode(const char*key, const bool &val, const Extend *ext) {
        if (!val && Extend::OmitEmpty(ext)) {
            return false;
        } else {
            std::string bval;
            if (val) {
                bval = "true";
            } else {
                bval = "false";
            }

            if (Extend::Attribute(ext)) {
                _cur->attrs.push_back(Attr(key, bval));
            } else {
                Node *n = new Node(key);
                n->val = bval;
                _cur->childs.push_back(n);
            }
        }
        return true;
    }

    #define XPACK_XML_ENCODE_INTEGER(type) \
    bool encode(const char*key, const type&val, const Extend*ext) {return this->encode_integer(key, val, ext);}
    XPACK_XML_ENCODE_INTEGER(char)
    XPACK_XML_ENCODE_INTEGER(signed char)
    XPACK_XML_ENCODE_INTEGER(unsigned char)
    XPACK_XML_ENCODE_INTEGER(short)
    XPACK_XML_ENCODE_INTEGER(unsigned short)
    XPACK_XML_ENCODE_INTEGER(int)
    XPACK_XML_ENCODE_INTEGER(unsigned int)
    XPACK_XML_ENCODE_INTEGER(long)
    XPACK_XML_ENCODE_INTEGER(unsigned long)
    XPACK_XML_ENCODE_INTEGER(long long)
    XPACK_XML_ENCODE_INTEGER(unsigned long long)

    #define XPACK_XML_ENCODE_FLOAT(type) \
    bool encode(const char*key, const type&val, const Extend*ext) {return this->encode_float(key, val, ext);}
    XPACK_XML_ENCODE_FLOAT(float)
    XPACK_XML_ENCODE_FLOAT(double)
    XPACK_XML_ENCODE_FLOAT(long double)

    // escape string
    static std::string StringQuote(const std::string &val) {
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

private:
    // integer
    template <class T>
    typename x_enable_if<numeric<T>::is_integer, bool>::type encode_integer(const char*key, const T& val, const Extend *ext) {
        if (val==0 && Extend::OmitEmpty(ext)) {
            return false;
        } else if (Extend::Attribute(ext)) {
            _cur->attrs.push_back(Attr(key, Util::itoa(val)));
        } else {
            Node *n = new Node(key);
            n->val = Util::itoa(val);
            _cur->childs.push_back(n);
        }
        return true;
    }

    // float
    template <class T>
    typename x_enable_if<numeric<T>::is_float, bool>::type encode_float(const char*key, const T &val, const Extend *ext) {
        if (val==0 && Extend::OmitEmpty(ext)) {
            return false;
        } else {
            std::ostringstream os;
            os<<val;
            std::string fval = os.str();

            if (Extend::Attribute(ext)) {
                _cur->attrs.push_back(Attr(key, fval));
            } else {
                Node *n = new Node(key);
                n->val = fval;
                _cur->childs.push_back(n);
            }
        }
        return true;
    }
    void appendNode(const Node *nd, int depth) {
        bool indentEnd = true;
        indent(depth);
        _output.push_back('<');
        _output += nd->key;
        std::list<Attr>::const_iterator it;
        for (it=nd->attrs.begin(); it!=nd->attrs.end(); ++it) {
            _output.push_back(' ');
            _output += it->key;
            _output += "=\"";
            _output += it->val;
            _output.push_back('"');
        }
        if (nd->val.empty() && nd->childs.size()==0) {
            _output += "/>";
            return;
        } else if (!nd->val.empty()) {
            _output.push_back('>');
            _output += nd->val;
            indentEnd = false;
        } else {
            _output.push_back('>');
            std::list<Node*>::const_iterator it;
            for (it=nd->childs.begin(); it!=nd->childs.end(); ++it) {
                appendNode(*it, depth+1);
            }
        }
        if (indentEnd) {
            indent(depth);
        }
        _output += "</";
        _output += nd->key;
        _output.push_back('>');
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
};

}

#endif
