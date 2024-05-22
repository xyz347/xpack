/*
* Copyright (C) 2021 Duowan Inc. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/

#ifndef __X_PACK_XML_DECODER_H
#define __X_PACK_XML_DECODER_H

#include <map>
#include <vector>
#include <fstream>

#include <cstdlib>
#include <string.h>

#include "rapidxml/rapidxml.hpp"

#include "xdecoder.h"

namespace xpack {

class XmlNode {
    typedef rapidxml::xml_node<> Node;
    typedef XDecoder<XmlNode> decoder;
public:
    typedef size_t Iterator;

public:
    XmlNode(Node *n=NULL):node(n), attr(NULL), inited(false) {}

    inline static const char * Name() {
        return "xml";
    }
    operator bool() const {
        return node != NULL;
    }
    bool IsNull() const {
        return false;
    }
    XmlNode Find(decoder&de, const char*key, const Extend *ext) {
        (void)de;
        if (!inited) {
            this->init();
        }
        if (Extend::XmlContent(ext)) {
            return *this;
        }

        node_index::iterator iter;
        if (_childs_index.end() != (iter=_childs_index.find(key))) {
            bool sbs = Extend::AliasFlag(ext, "xml", "sbs");
            if (!sbs) {
                return XmlNode(_childs[iter->second]);
            } else {
                XmlNode node(_childs[iter->second]);
                node.initsbs(*this, key);
                return node;
            }
        } else { // sbs not support attribute
            XmlNode tmp;
            tmp.attr = node->first_attribute(key);
            if (NULL != tmp.attr) {
                tmp.node = this->node;
            }
            return tmp;
        }
    }
    size_t Size(decoder&de) {
        (void)de;
        if (!inited) {
            this->init();
        }
        return _childs.size();
    }
    XmlNode At(size_t index) const { // no exception
        return XmlNode(_childs[index]);
    }
    // if child node defined sbs, results can be confusing
    XmlNode Next(decoder&de, XmlNode&p, Iterator&iter, std::string&key) {
        (void)de;
        if (!p.inited) {
            p.init();
        }
        if (node != NULL) {
            if (node != p.node) {
                ++iter;
            } else {
                iter = 0;
            }

            if (iter < p._childs.size()) {
                key = p._childs[iter]->name();
                return XmlNode(p._childs[iter]);
            }
        }
        return XmlNode();
    }
    bool Get(decoder&de, std::string&val, const Extend*ext) {
        if (!Extend::AliasFlag(ext, "xml", "cdata")) {
            val = get_val(Extend::XmlContent(ext));
        } else {
            const Node *tmp = node->first_node();
            if (NULL != tmp) {
                if (tmp->type() == rapidxml::node_cdata || tmp->type() == rapidxml::node_data) {
                    val = tmp->value();
                } else {
                    de.decode_exception("not cdata type", NULL);
                }
            } else { // if node contain text not CDATA, get it
                val = node->value();
            }
        }
        return true;
    }
    bool Get(decoder&de, bool &val, const Extend*ext) {
        (void)ext;
        std::string v = get_val(Extend::XmlContent(ext));
        if (v=="1" || v=="true" || v=="TRUE" || v=="True") {
            val = true;
        } else if (v=="0" || v=="false" || v=="FALSE" || v=="False") {
            val = false;
        } else {
            de.decode_exception("parse bool fail.", NULL);
        }
        return true;
    }
    template <class T>
    typename x_enable_if<numeric<T>::is_integer, bool>::type Get(decoder&de, T &val, const Extend*ext){
        (void)ext;
        std::string v = get_val(Extend::XmlContent(ext));
        if (Util::atoi(v, val)) {
            return true;
        } else {
            de.decode_exception("parse int fail. not integer or overflow", NULL);
            return false;
        }
    }
    template <class T>
    typename x_enable_if<numeric<T>::is_float, bool>::type Get(decoder&de, T &val, const Extend*ext){
        (void)ext;
        std::string v = get_val(Extend::XmlContent(ext));
        if (1==v.length() && v[0]=='-') {
            return false;
        }

        const char *data = v.c_str();
        char *end;
        double d = strtod(data, &end);
        if ((size_t)(end-data) == v.length()) {
            val = (T)d;
            return true;
        }
        de.decode_exception("parse double fail.", NULL);
        return false;
    }

private:
    typedef std::map<const char*, size_t, cmp_str> node_index; // index of _childs

    void init() {
        inited = true;
        if (NULL != node) {
            Node *tmp = node->first_node();
            for (size_t i=0; tmp; tmp=tmp->next_sibling(), ++i) {
                _childs.push_back(tmp);
                _childs_index[tmp->name()] = i;
            }
        }
    }
    void initsbs(const XmlNode&parent, const char *key) {
        inited = true;
        for (size_t i=0; i<parent._childs.size(); ++i) {
            if (0 == strcmp(key, parent._childs[i]->name())) {
                _childs.push_back(parent._childs[i]);
            }
        }
    }

    std::string get_val(bool forceContent=false) {
        if (forceContent || attr==NULL) {
            return node->value();
        } else if  (attr != NULL) {
            return attr->value();
        } else {
            return "";
        }
    }

    const Node* node;   // current node
    rapidxml::xml_attribute<char> *attr;
    bool inited;        // delay init to avoid copy _childs and _childs_index

    std::vector<Node*> _childs;  // childs
    node_index _childs_index;
};


class XmlDecoder {
public:
    template <class T>
    bool decode(const std::string&str, T&val, bool with_root=false) {
        std::string tmp = str;
        return this->decode_indata(tmp, val, with_root);
    }
    template <class T>
    bool decode_file(const std::string&fname, T&val, bool with_root=false) {
        std::string data;
        bool ret = Util::readfile(fname, data);
        if (ret) {
            ret = this->decode_indata(data, val, with_root);
        }
        return ret;
    }
private:
    template <class T>
    bool decode_indata(std::string&str, T&val, bool with_root=false) {
        rapidxml::xml_document<> de;
        std::string err;
        try {
            de.parse<0>((char*)str.c_str());
        } catch (const rapidxml::parse_error&e) {
            err = std::string("parse xml fail. err=")+e.what()+". "+std::string(e.where<char>()).substr(0, 32);
        } catch (const std::exception&e) {
            err = std::string("parse xml fail. unknow exception. err=")+e.what();
        }

        if (!err.empty()) {
            throw std::runtime_error(err);
        }

        if (!with_root) {
            XmlNode node(de.first_node());
            return XDecoder<XmlNode>(NULL, (const char*)NULL, node).decode(val, NULL);
        } else { 
            XmlNode node(&de);
            return XDecoder<XmlNode>(NULL, (const char*)NULL, node).decode(val, NULL);
        }
    }
};

}

#endif
