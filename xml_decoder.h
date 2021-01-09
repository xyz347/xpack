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

#include "thirdparty/rapidxml/rapidxml.hpp"

#include "xdecoder.h"

namespace xpack {

class XmlDecoder:public XDecoder<XmlDecoder> {
    typedef rapidxml::xml_document<> XML_READER_DOCUMENT;
    typedef rapidxml::xml_node<> XML_READER_NODE;  
public:
    friend class XDecoder<XmlDecoder>;
    using xdoc_type::decode;

    XmlDecoder(const std::string& str, bool isfile=false):xdoc_type(NULL, ""),_doc(new XML_READER_DOCUMENT),_node(NULL) {
        std::string err;
        _xml_data = NULL;

        do {
            try {
                if (isfile) {
                    std::ifstream fs(str.c_str(), std::ifstream::binary);
                    if (!fs) {
                        err = "Open file["+str+"] fail.";
                        break;
                    }
                    std::string data((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
                    _xml_data = new char[data.length()+1];
                    memcpy(_xml_data, data.data(), data.length());
                    _xml_data[data.length()] = '\0';
                } else  {
                    _xml_data = new char[str.length()+1];
                    memcpy(_xml_data, str.data(), str.length());
                    _xml_data[str.length()] = '\0';
                }
                _doc->parse<0>(_xml_data);
            } catch (const rapidxml::parse_error&e) {
                err = std::string("parse xml fail. err=")+e.what()+". "+std::string(e.where<char>()).substr(0, 32);
            } catch (const std::exception&e) {
                err = std::string("parse xml fail. unknow exception. err=")+e.what();
            }

            if (!err.empty()) {
                break;
            }

            _node = _doc->first_node(); // root

            init();
            return;
        } while (false);

        delete _doc;
        _doc = NULL;
        if (NULL != _xml_data) {
            delete []_xml_data;
            _xml_data = NULL;
        }
        throw std::runtime_error(err);
    }
    ~XmlDecoder() {
        if (NULL != _doc) {
            delete _doc;
            delete []_xml_data;
            _doc = NULL;
            _xml_data = NULL;
        }
    }

    const char * Type() {
        return "xml";
    }

public: // decode
    #define XPACK_XML_DECODE_CHECK()                              \
        bool exists; std::string v = get_val(key, exists);        \
        if (!exists) {                                            \
            if (Extend::Mandatory(ext)) {                         \
                decode_exception("mandatory key not found", key); \
            }                                                     \
            return false;                                         \
        }

    #define XPACK_XML_DECODE_INTEGER(type) \
    bool decode(const char*key, type&val, const Extend*ext) {return this->decode_integer(key, val, ext);}
    XPACK_XML_DECODE_INTEGER(char)
    XPACK_XML_DECODE_INTEGER(signed char)
    XPACK_XML_DECODE_INTEGER(unsigned char)
    XPACK_XML_DECODE_INTEGER(short)
    XPACK_XML_DECODE_INTEGER(unsigned short)
    XPACK_XML_DECODE_INTEGER(int)
    XPACK_XML_DECODE_INTEGER(unsigned int)
    XPACK_XML_DECODE_INTEGER(long)
    XPACK_XML_DECODE_INTEGER(unsigned long)
    XPACK_XML_DECODE_INTEGER(long long)
    XPACK_XML_DECODE_INTEGER(unsigned long long)

    // string
    bool decode(const char *key, std::string &val, const Extend *ext) {
        XPACK_XML_DECODE_CHECK();
        val = v;
        return true;
    }
    bool decode(const char *key, bool &val, const Extend *ext) {
        XPACK_XML_DECODE_CHECK();
        if (v=="1" || v=="true" || v=="TRUE" || v=="True") {
            val = true;
        } else if (v=="0" || v=="false" || v=="FALSE" || v=="False") {
            val = false;
        } else {
			decode_exception("parse bool fail.", key);
            return false;
        }
		return true;
    }

    bool decode(const char *key, double &val, const Extend *ext) {
        XPACK_XML_DECODE_CHECK();
		if (1==v.length() && v[0]=='-') {
			return false;
		}
		
		const char *data = v.c_str();
		char *end;
		double d = strtod(data, &end);
		if ((size_t)(end-data) == v.length()) {
			val = d;
			return true;
		}
		decode_exception("parse double fail.", key);
		return false;
    }
    bool decode(const char *key, float &val, const Extend *ext) {
		double d;
        bool ret = this->decode(key, d, ext);
		if (ret) {
			val = (float)d;
		}
		return ret;
    }
    bool decode(const char *key, long double &val, const Extend *ext) {
		double d;
        bool ret = this->decode(key, d, ext);
		if (ret) {
			val = (long double)d;
		}
		return ret;
    }

    size_t Size() {
            return _childs.size();
    }
    XmlDecoder At(size_t index) {
        if (index < _childs.size()) {
            return XmlDecoder(_childs[index], this, index);
        } else {
            throw std::runtime_error("Out of index");
        }
        return XmlDecoder(NULL, NULL, "");
    }
    XmlDecoder* Find(const char*key, XmlDecoder*tmp) {
        node_index::iterator iter;
        if (_childs_index.end() != (iter=_childs_index.find(key))) {
            tmp->_key = key;
            tmp->_parent = this;
            tmp->_node = _childs[iter->second];
            tmp->init();
            return tmp;
        } else {
            return NULL;
        }
    }
    XmlDecoder Begin() {
        if (_childs.size() > 0) {
            return XmlDecoder(_childs[0], this, _childs[0]->name(), 0);
        } else {
            return XmlDecoder(NULL, this, "");
        }
    }
    XmlDecoder Next() {
        if (NULL == _parent) {
            throw std::runtime_error("parent null");
        } else {
            size_t iter = _iter+1;
            if (iter < _parent->_childs.size()) {
                return XmlDecoder(_parent->_childs[iter], _parent, _parent->_childs[iter]->name(), iter);
            } else {
                return XmlDecoder(NULL, _parent, "");
            }
        }
    }
    operator bool() const {
        return NULL != _node;
    }

private:
	// integer. if we named this as decode, clang and msvc will fail
    template <class T>
    typename x_enable_if<numeric<T>::is_integer, bool>::type decode_integer(const char *key, T &val, const Extend *ext) {
        XPACK_XML_DECODE_CHECK();
		if (Util::atoi(v, val)) {
            return true;
        } else {
            decode_exception("parse int fail. not integer or overflow", key);
            return false;
        }
    }

    typedef std::map<const char*, size_t, cmp_str> node_index; // index of _childs

    XmlDecoder():xdoc_type(NULL, ""),_doc(NULL),_node(NULL) {
        init();
    }
    XmlDecoder(const XML_READER_NODE* val, const XmlDecoder*parent, const char*key, size_t iter=0):xdoc_type(parent, key),_doc(NULL),_node(val),_iter(iter) {
        init();
    }
    XmlDecoder(const XML_READER_NODE* val, const XmlDecoder*parent, size_t index):xdoc_type(parent, index),_doc(NULL),_node(val) {
        init();
    }
    void init() {
        if (NULL != _node) {
            XML_READER_NODE *tmp = _node->first_node();
            for (size_t i=0; tmp; tmp=tmp->next_sibling(), ++i) {
                _childs.push_back(tmp);
                _childs_index[tmp->name()] = i;
            }
        }
    }

    std::string get_val(const char *key, bool &exists) {
        exists = true;
        if (NULL == key) {
            return _node->value();
        } else {
            node_index::iterator iter;
            if (_childs_index.end()!=(iter=_childs_index.find(key))) {
                return _childs[iter->second]->value();
            } else {
                rapidxml::xml_attribute<char> *attr = _node->first_attribute(key);
                if (NULL != attr) {
                    return attr->value();
                }
            }
        }
        exists = false;
        return "";
    }

    // for parse xml file. only root has this
    XML_READER_DOCUMENT* _doc;
    char *_xml_data;

    const XML_READER_NODE* _node;           // current node
    std::vector<XML_READER_NODE*> _childs;  // childs
    node_index _childs_index;
    size_t _iter;
};

}

#endif
