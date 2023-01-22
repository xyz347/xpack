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

#include <iostream>

#ifdef XGTEST
#include<gtest/gtest.h>
#else
#include "gtest_stub.h"
#endif

#include "xpack/json.h"
#include "xpack/xml.h"
#include "string.h"

using namespace std;

struct Base {
    int    a;
    string b;
    Base(const int _a=0, const string&_b=""):a(_a), b(_b){}
#ifndef XPACK_OUT_TEST
    XPACK(O(a, b));
};
#else
};
XPACK_OUT(Base, O(a,b));
#endif


struct POD {
    int  a;
    char b[10];
#ifndef XPACK_OUT_TEST
    XPACK(O(a, b));
};
#else
};
XPACK_OUT(POD, O(a,b));
#endif

// ++++++++++++++++++++++BuiltInTypes++++++++++++++++++++++++++++
struct BuiltInTypes {
    signed char        sch;
    char               ch;
    unsigned char      uch;
    short              sh;
    unsigned short     ush;
    int                i;
    unsigned int       ui;
    long               l;
    unsigned long      ul;
    long long          ll;
    unsigned long long ull;
    float              f;
    double             d;
    long double        ld;
    bool               b;
#ifndef XPACK_OUT_TEST
    XPACK(AF(F(ATTR), sch, "xml:s:ch"), X(F(ATTR), ch, uch, sh, ush), O(i, ui, l, ul, ll, ull, f, d, ld, b));
};
#else
};
XPACK_OUT(BuiltInTypes, AF(F(ATTR), sch, "xml:s:ch"), X(F(ATTR), ch, uch, sh, ush), O(i, ui, l, ul, ll, ull, f, d, ld, b));
#endif

void checkBuiltInTypes(const BuiltInTypes&a, const BuiltInTypes&b) {
    EXPECT_EQ(a.sch, b.sch);
    EXPECT_EQ(a.ch, b.ch);
    EXPECT_EQ(a.uch, b.uch);
    EXPECT_EQ(a.sh, b.sh);
    EXPECT_EQ(a.ush, b.ush);
    EXPECT_EQ(a.i, b.i);
    EXPECT_EQ(a.ui, b.ui);
    EXPECT_EQ(a.l, b.l);
    EXPECT_EQ(a.ul, b.ul);
    EXPECT_EQ(a.ll, b.ll);
    EXPECT_EQ(a.ull, b.ull);
    EXPECT_FLOAT_EQ(a.f, b.f);
    EXPECT_DOUBLE_EQ(a.d, b.d);
    EXPECT_DOUBLE_EQ(a.ld, b.ld);
    EXPECT_TRUE(a.b);
}
TEST(builtin, types) {
    BuiltInTypes bt;
    bt.sch = 0x7f;
    bt.ch = 0x7f;
    bt.uch = 0x80;
    bt.sh = 0x7fff;
    bt.ush = 0x8000;
    bt.i = 0x7fffffff;
    bt.ui = 0x80000000;
    bt.l =  0x7fffffffffffffff;
    bt.ul = 0x8000000000000000;
    bt.ll = bt.l;
    bt.ull = bt.ul;
    bt.f = 1.234;
    bt.d = 9.678;
    bt.ld = 30.678;
    bt.b = true;

    string s1 = xpack::json::encode(bt);
    BuiltInTypes j1;
    xpack::json::decode(s1, j1);
    checkBuiltInTypes(j1, bt);

    string s2 = xpack::xml::encode(bt, "root");
    BuiltInTypes j2;
    xpack::xml::decode(s2, j2);
    checkBuiltInTypes(j2, bt);
}

// ++++++++++++++++++++bit field+++++++++++++++++++++++++++
struct BitField {
    short a:8;
    short b:8;
#ifndef XPACK_OUT_TEST
    XPACK(B(F(0), a, b));
};
#else
};
XPACK_OUT(BitField, B(F(0), a, b));
#endif
void checkBitField(const BitField&a, const BitField&b) {
    EXPECT_EQ(a.a, b.a);
    EXPECT_EQ(a.b, b.b);
}
TEST(bitfiled, base) {
    BitField bf;
    bf.a = 0x70;
    bf.b = 0x7f;

    string s1 = xpack::json::encode(bf);
    BitField j1;
    xpack::json::decode(s1, j1);
    checkBitField(j1, bf);

    string s2 = xpack::xml::encode(bf, "root");
    BitField j2;
    xpack::xml::decode(s2, j2);
    checkBitField(j2, bf);
}

// +++++++++++++++++++ enum +++++++++++++++++++++
enum Enum1 {
    E0 = 0,
    E1 = 1,
};
#ifdef X_PACK_SUPPORT_CXX0X
enum class Enum2:int64_t {
    E2 = 2,
    E3 = 3,
};
#else
enum Enum2 {
    E2 = 2,
    E3 = 3,
};
#endif
struct EnumBase {
    Enum1 e1;
    Enum2 e2;
#ifndef XPACK_OUT_TEST
    #ifdef X_PACK_SUPPORT_CXX0X
    XPACK(O(e1), E(F(0), e2));
    #else
    XPACK(E(F(0), e1, e2));
    #endif
};
#else
};
#ifdef X_PACK_SUPPORT_CXX0X
XPACK_OUT(EnumBase, O(e1), E(F(0), e2));
#else
XPACK_OUT(EnumBase, E(F(0), e1, e2));
#endif
#endif
void checkEnumBase(const EnumBase&a, const EnumBase&b) {
    EXPECT_EQ(a.e1, b.e1);
    EXPECT_EQ(a.e2, b.e2);
}
TEST(enum, base) {
    EnumBase eb;
    eb.e1 = E0;
#ifdef X_PACK_SUPPORT_CXX0X
    eb.e2 = Enum2::E3;
#else
    eb.e2 = E3;
#endif

    string s1 = xpack::json::encode(eb);
    EnumBase j1;
    xpack::json::decode(s1, j1);
    checkEnumBase(j1, eb);

    string s2 = xpack::xml::encode(eb, "root");
    EnumBase j2;
    xpack::xml::decode(s2, j2);
    checkEnumBase(j2, eb);
}
#ifdef X_PACK_SUPPORT_CXX0X
TEST(enum, map) {
    map<Enum2, int> m;
    m[Enum2::E2] = 2;
    m[Enum2::E3] = 3;

    string s1 = xpack::json::encode(m);
    map<Enum2, int> j1;
    xpack::json::decode(s1, j1);
    EXPECT_EQ(j1[Enum2::E2], 2);
    EXPECT_EQ(j1[Enum2::E3], 3);
}
#endif

// ++++++++++++++++inherit+++++++++++++++++++++++++++++
struct InheritBase {
    int b1;
    string b2;
#ifndef XPACK_OUT_TEST
    XPACK(O(b1, b2));
};
#else
};
XPACK_OUT(InheritBase, O(b1, b2));
#endif
struct InheritChild:public InheritBase {
    int c1;
    string c2;
#ifndef XPACK_OUT_TEST
    XPACK(I(InheritBase), O(c1, c2));
};
#else
};
XPACK_OUT(InheritChild, I(InheritBase), O(c1, c2));
#endif
void checkInherit(const InheritChild&a, const InheritChild&b) {
    EXPECT_EQ(a.b1, b.b1);
    EXPECT_EQ(a.b2, b.b2);
    EXPECT_EQ(a.c1, b.c1);
    EXPECT_EQ(a.c2, b.c2);
}
TEST(inherit, base) {
    InheritChild c;
    c.b1 = 1;
    c.b2 = "base";
    c.c1 = 2;
    c.c2 = "child";

    string s1 = xpack::json::encode(c);
    InheritChild j1;
    xpack::json::decode(s1, j1);
    checkInherit(j1, c);

    string s2 = xpack::xml::encode(c, "root");
    InheritChild j2;
    xpack::xml::decode(s2, j2);
    checkInherit(j2, c);
}

// ++++++++++++++++++++++++array++++++++++++++++++++++++++
struct Array {
    int  a[3];
    char b[3];
    Base c[3];
#ifndef XPACK_OUT_TEST
    XPACK(O(a, b, c));
};
#else
};
XPACK_OUT(Array, O(a, b, c));
#endif
void checkArray(const Array&a, const Array&b) {
    EXPECT_EQ(a.a[0], b.a[0]);
    EXPECT_EQ(a.a[1], b.a[1]);
    EXPECT_EQ(a.a[2], b.a[2]);
    EXPECT_EQ(a.b[0], b.b[0]);
    EXPECT_EQ(a.b[1], b.b[1]);
    EXPECT_EQ(a.b[2], b.b[2]);
    EXPECT_EQ(a.c[0].a, b.c[0].a);
    EXPECT_EQ(a.c[0].b, b.c[0].b);
    EXPECT_EQ(a.c[1].a, b.c[1].a);
    EXPECT_EQ(a.c[1].b, b.c[1].b);
    EXPECT_EQ(a.c[2].a, b.c[2].a);
    EXPECT_EQ(a.c[2].b, b.c[2].b);
}
TEST(array, base) {
    Array a;
    a.a[0] = 1;
    a.a[1] = 2;
    a.a[2] = 3;
    a.b[0] = 'g';
    a.b[1] = 'o';
    a.b[2] = '\0';
    a.c[0].a = 11;
    a.c[1].a = 12;
    a.c[2].a = 13;
    a.c[0].b = "hello";
    a.c[1].b = "good";
    a.c[1].b = "nice";

    string s1 = xpack::json::encode(a);
    Array j1;
    xpack::json::decode(s1, j1);
    checkArray(j1, a);

    string s2 = xpack::xml::encode(a, "root");
    Array j2;
    xpack::xml::decode(s2, j2);
    checkArray(j2, a);

    char ca[3];
    xpack::json::decode("\"good\"", ca);
    EXPECT_TRUE(0==strcmp(ca, "go"));

    int ia[3];
    xpack::json::decode("[1, 2, 3, 4]", ia);
    EXPECT_EQ(ia[2], 3);
}


// +++++++++++++++ container ++++++++++++++++++++++
struct ContainerBase {
    map<string, int> m;
    set<int> s;
    vector<int> v;
    list<int> l;

    vector<vector<int> > vv;
#ifndef XPACK_OUT_TEST
    XPACK(O(m, s, v, l, vv));
};
#else
};
XPACK_OUT(ContainerBase, O(m, s, v, l, vv));
#endif
void checkContainerBase(ContainerBase&cb) {
    EXPECT_EQ(cb.m.size(), 2U);
    EXPECT_EQ(cb.m["a"], 1);
    EXPECT_EQ(cb.m["b"], 2);
    EXPECT_EQ(cb.s.size(), 2U);
    EXPECT_TRUE(cb.s.find(3)!=cb.s.end());
    EXPECT_TRUE(cb.s.find(4)!=cb.s.end());
    EXPECT_TRUE(cb.s.find(5)==cb.s.end());
    EXPECT_EQ(cb.v.size(), 2U);
    EXPECT_EQ(cb.v[0], 5);
    EXPECT_EQ(cb.v[1], 6);
    EXPECT_EQ(cb.l.size(), 2U);
    EXPECT_EQ(*(cb.l.begin()), 7);
    EXPECT_EQ(*(++cb.l.begin()), 8);

    EXPECT_EQ(cb.vv.size(), 2U);
    EXPECT_EQ(cb.vv[0].size(), 2U);
    EXPECT_EQ(cb.vv[1].size(), 2U);
    EXPECT_EQ(cb.vv[0][0], 11);
    EXPECT_EQ(cb.vv[0][1], 12);
    EXPECT_EQ(cb.vv[1][0], 22);
    EXPECT_EQ(cb.vv[1][1], 23);
}
TEST(container, base) {
    string s = "{\"m\":{\"a\":1, \"b\":2}, \"s\":[3, 4], \"v\":[5,6], \"l\":[7, 8], \"vv\":[[11,12],[22,23]]}";

    ContainerBase cb;
    xpack::json::decode(s, cb);
    checkContainerBase(cb);

    string s1 = xpack::json::encode(cb);
    ContainerBase cb2;
    xpack::json::decode(s1, cb2);
    string s2 = xpack::json::encode(cb2);
    EXPECT_EQ(s1, s2);

    string s3 = xpack::xml::encode(cb, "root");
    ContainerBase cb3;
    xpack::xml::decode(s3, cb3);
    checkContainerBase(cb3);
    string s4 = xpack::xml::encode(cb3, "root");
    EXPECT_EQ(s3, s4);
}
struct ContainerStruct {
    map<string, Base> m;
    vector<Base> v;
    list<Base> l;

    vector<vector<Base> > vv;
#ifndef XPACK_OUT_TEST
    XPACK(O(m, v, l, vv));
};
#else
};
XPACK_OUT(ContainerStruct, O(m, v, l, vv));
#endif
void checkContainerStruct(ContainerStruct&a) {
    EXPECT_EQ(a.m.size(), 2U);
    EXPECT_EQ(a.m["a"].a, 1);
    EXPECT_EQ(a.m["a"].b, "good");
    EXPECT_EQ(a.m["b"].a, 2);
    EXPECT_EQ(a.m["b"].b, "nice");

    EXPECT_EQ(a.v.size(), 2U);
    EXPECT_EQ(a.v[0].a, 3);
    EXPECT_EQ(a.v[0].b, "hello");
    EXPECT_EQ(a.v[1].a, 4);
    EXPECT_EQ(a.v[1].b, "wow");

    list<Base>::const_iterator lit = a.l.begin();
    EXPECT_EQ(a.l.size(), 2U);
    EXPECT_EQ(lit->a, 5);
    EXPECT_EQ(lit->b, "dida");
    ++lit;
    EXPECT_EQ(lit->a, 6);
    EXPECT_EQ(lit->b, "haha");

    EXPECT_EQ(a.vv.size(), 2U);
    EXPECT_EQ(a.vv[0].size(), 1U);
    EXPECT_EQ(a.vv[1].size(), 2U);
    EXPECT_EQ(a.vv[0][0].a, 7);
    EXPECT_EQ(a.vv[0][0].b, "lala");
    EXPECT_EQ(a.vv[1][0].a, 8);
    EXPECT_EQ(a.vv[1][0].b, "wawa");
    EXPECT_EQ(a.vv[1][1].a, 9);
    EXPECT_EQ(a.vv[1][1].b, "kaka");
}
TEST(container, struct) {
    ContainerStruct cb;
    cb.m["a"] = Base(1, "good");
    cb.m["b"] = Base(2, "nice");
    cb.v.push_back(Base(3, "hello"));
    cb.v.push_back(Base(4, "wow"));
    cb.l.push_back(Base(5, "dida"));
    cb.l.push_back(Base(6, "haha"));
    cb.vv.resize(2);
    cb.vv[0].resize(1);
    cb.vv[1].resize(2);
    cb.vv[0][0].a = 7;
    cb.vv[0][0].b = "lala";
    cb.vv[1][0].a = 8;
    cb.vv[1][0].b = "wawa";
    cb.vv[1][1].a = 9;
    cb.vv[1][1].b = "kaka";

    string s1 = xpack::json::encode(cb);
    ContainerStruct cb1;
    xpack::json::decode(s1, cb1);
    checkContainerStruct(cb1);
    string s2 = xpack::json::encode(cb1);
    EXPECT_EQ(s1, s2);

    string s3 = xpack::xml::encode(cb, "root");
    ContainerStruct cb3;
    xpack::xml::decode(s3, cb3);
    checkContainerStruct(cb3);
    string s4 = xpack::xml::encode(cb3, "root");
    EXPECT_EQ(s3, s4);
}

// +++++++++++++++++++ QT ++++++++++++++++++++
#ifdef XPACK_SUPPORT_QT
struct ContainerQT {
    QMap<string, Base> m;
    QVector<Base> v;
    QList<Base> l;

    QVector<QVector<Base> > vv;
#ifndef XPACK_OUT_TEST
    XPACK(O(m, v, l, vv));
};
#else
};
XPACK_OUT(ContainerQT, O(m, v, l, vv));
#endif
void checkContainerQT(ContainerQT&a) {
    EXPECT_EQ(a.m.size(), 2);
    EXPECT_EQ(a.m["a"].a, 1);
    EXPECT_EQ(a.m["a"].b, "good");
    EXPECT_EQ(a.m["b"].a, 2);
    EXPECT_EQ(a.m["b"].b, "nice");

    EXPECT_EQ(a.v.size(), 2);
    EXPECT_EQ(a.v[0].a, 3);
    EXPECT_EQ(a.v[0].b, "hello");
    EXPECT_EQ(a.v[1].a, 4);
    EXPECT_EQ(a.v[1].b, "wow");

    QList<Base>::const_iterator lit = a.l.begin();
    EXPECT_EQ(a.l.size(), 2);
    EXPECT_EQ(lit->a, 5);
    EXPECT_EQ(lit->b, "dida");
    ++lit;
    EXPECT_EQ(lit->a, 6);
    EXPECT_EQ(lit->b, "haha");

    EXPECT_EQ(a.vv.size(), 2);
    EXPECT_EQ(a.vv[0].size(), 1);
    EXPECT_EQ(a.vv[1].size(), 2);
    EXPECT_EQ(a.vv[0][0].a, 7);
    EXPECT_EQ(a.vv[0][0].b, "lala");
    EXPECT_EQ(a.vv[1][0].a, 8);
    EXPECT_EQ(a.vv[1][0].b, "wawa");
    EXPECT_EQ(a.vv[1][1].a, 9);
    EXPECT_EQ(a.vv[1][1].b, "kaka");
}
TEST(container, QT) {
    ContainerQT cb;
    cb.m["a"] = Base(1, "good");
    cb.m["b"] = Base(2, "nice");
    cb.v.push_back(Base(3, "hello"));
    cb.v.push_back(Base(4, "wow"));
    cb.l.push_back(Base(5, "dida"));
    cb.l.push_back(Base(6, "haha"));
    cb.vv.resize(2);
    cb.vv[0].resize(1);
    cb.vv[1].resize(2);
    cb.vv[0][0].a = 7;
    cb.vv[0][0].b = "lala";
    cb.vv[1][0].a = 8;
    cb.vv[1][0].b = "wawa";
    cb.vv[1][1].a = 9;
    cb.vv[1][1].b = "kaka";

    string s1 = xpack::json::encode(cb);
    ContainerQT cb1;
    xpack::json::decode(s1, cb1);
    checkContainerQT(cb1);
    string s2 = xpack::json::encode(cb1);
    EXPECT_EQ(s1, s2);

    string s3 = xpack::xml::encode(cb, "root");
    ContainerQT cb3;
    xpack::xml::decode(s3, cb3);
    checkContainerQT(cb3);
    string s4 = xpack::xml::encode(cb3, "root");
    EXPECT_EQ(s3, s4);
}
#endif

// ++++++++++++++ xtypes +++++++++++++++++++++
struct XtypeUnion {
    int type;
    union {
        POD  p;
        int  i;
        char s[10];
    };
};
struct XtypeUnionTop {
    string name;
    XtypeUnion un;
#ifndef XPACK_OUT_TEST
    XPACK(O(name, un));
};
#else
};
XPACK_OUT(XtypeUnionTop, O(name, un));
#endif

namespace xpack { // must define in namespace xpack

template<>
struct is_xpack_xtype<XtypeUnion> {static bool const value = true;};

template<class OBJ>
bool xpack_xtype_decode(OBJ &obj, const char*key, XtypeUnion &val, const Extend *ext) {
    OBJ *o = obj.find(key, ext);
    if (NULL == o) {
        // should check Mandatory
        return false;
    }
    o->decode("type", val.type, NULL);
    switch (val.type) {
        case 1:
            o->decode("p", val.p, NULL);
            break;
        case 2:
            o->decode("i", val.i, NULL);
            break;
        case 3:
            o->decode("s", val.s, NULL);
            break;
    }
    return true;
}

template<class OBJ>
bool xpack_xtype_encode(OBJ &obj, const char*key, const XtypeUnion &val, const Extend *ext) {
    obj.ObjectBegin(key, ext);
    obj.encode("type", val.type, NULL);
    switch (val.type) {
        case 1:
            obj.encode("p", val.p, NULL);
            break;
        case 2:
            obj.encode("i", val.i, NULL);
            break;
        case 3:
            obj.encode("s", val.s, NULL);
            break;
    }
    obj.ObjectEnd(key, ext);
    return true;
}

}

void checkXtypesUnion(const XtypeUnionTop &xt, bool cn) {
    if (cn) {
        EXPECT_EQ(xt.name, "hello");
    }

    if (xt.un.type == 1) {
        EXPECT_EQ(xt.un.p.a, 10);
        EXPECT_TRUE(0 == strcmp(xt.un.p.b, "good"));
    } else if (xt.un.type == 2) {
        EXPECT_EQ(xt.un.i, 20);
    } else if (xt.un.type == 3) {
        EXPECT_TRUE(0 == strcmp(xt.un.s, "nice"));
    } else {
        EXPECT_EQ(xt.un.type, 1);
    }
}
TEST(xtypes, union) {
    XtypeUnionTop xt;
    xt.name = "hello";
    xt.un.type = 1;
    xt.un.p.a = 10;
    strcpy(xt.un.p.b, "good");

    string s = xpack::json::encode(xt);
    XtypeUnionTop xt1;
    xpack::json::decode(s, xt1);
    checkXtypesUnion(xt1, true);

    s = xpack::xml::encode(xt, "root");
    XtypeUnionTop xt2;
    xpack::xml::decode(s, xt2);
    checkXtypesUnion(xt2, true);

    s = xpack::json::encode(xt.un);
    XtypeUnionTop xt3;
    xpack::json::decode(s, xt3.un);
    checkXtypesUnion(xt3, false);

    s = xpack::xml::encode(xt.un, "root");
    XtypeUnionTop xt4;
    xpack::xml::decode(s, xt4.un);
    checkXtypesUnion(xt4, false);
}

// +++++++++++++++++ custom +++++++++++++++++
struct Custom {
    int a;
    int b;
    int c;
    XPACK(O(a, b), C(hex, F(0), c));
};

namespace xpack {
template <class OBJ>
bool hex_encode(OBJ &obj, const Custom&c, const char*key, const int &i, const Extend *ext) {
    (void)c;
    stringstream ss;
    ss<<"0x"<<std::hex<<i;
    return obj.encode(key, ss.str(), ext);
}
template <class OBJ>
bool hex_decode(OBJ &obj, Custom&c, const char*key, int &i, const Extend *ext) {
    (void)c;
    string s;
    if (!obj.decode(key, s, ext)) {
        return false;
    }
    stringstream ss;
    ss<<std::hex<<s;
    ss>>i;
    return true;
}

}
TEST(custom, hex) {
    Custom c;
    c.a = 1;
    c.b = 2;
    c.c = 0xe;
    string s = xpack::json::encode(c);
    EXPECT_EQ(s, "{\"a\":1,\"b\":2,\"c\":\"0xe\"}");

    Custom c1;
    xpack::json::decode(s, c1);
    EXPECT_EQ(c1.a, 1);
    EXPECT_EQ(c1.b, 2);
    EXPECT_EQ(c1.c, 0xe);
}

// +++++++++++++++++ flags ++++++++++++++++++
struct FlagEN {
    int a;
    string b;
#ifdef X_PACK_SUPPORT_CXX0X
    shared_ptr<int> c;
#else
    int c;
#endif
    XPACK(X(F(EN), a, b, c));
    FlagEN(){c=0;}
};
TEST(flags, EN) {
    FlagEN fe;
    fe.a = 1;

    string s = xpack::json::encode(fe);
    EXPECT_EQ(s, "{\"a\":1,\"b\":null,\"c\":null}");

    FlagEN f1;
    xpack::json::decode(s, f1);
    EXPECT_EQ(f1.a, 1);
    EXPECT_EQ(f1.b, "");
#ifdef X_PACK_SUPPORT_CXX0X
    EXPECT_EQ(f1.c.get(), (int*)NULL);
#else
    EXPECT_EQ(f1.c, 0);
#endif
}
struct FlagM {
    int a;
    string b;
    XPACK(O(a), M(b));
};
TEST(flags, M) {
    string s = "{\"a\":1}";
    bool except = false;
    try {
        FlagM f;
        xpack::json::decode(s, f);
    } catch(...) {
        except = true;
    }
    EXPECT_TRUE(except);
}
struct FlagATTR {
    int a;
    string b;
    XPACK(O(a), X(F(ATTR), b));
};
TEST(flags, ATTR) {
    FlagATTR f;
    f.a = 1;
    f.b = "hello";
    string s = xpack::xml::encode(f, "root");
    EXPECT_EQ(s, "<root b=\"hello\"><a>1</a></root>");

    FlagATTR f1;
    xpack::xml::decode(s, f1);
    EXPECT_EQ(f1.a, 1);
    EXPECT_EQ(f1.b, "hello");
}
struct FlagVectorLabel {
    vector<int> a;
    vector<int> b;
    vector<int> c;
    XPACK(O(a), A(b, "xml:b,sbs", c, "xml:c,vl@x"));
};
TEST(flags, vectorlabel) {
    FlagVectorLabel f;
    f.a.push_back(1);
    f.a.push_back(2);
    f.b.push_back(3);
    f.b.push_back(4);
    f.c.push_back(5);
    f.c.push_back(6);

    string s = xpack::xml::encode(f, "root");
    EXPECT_EQ(s, "<root><a><a>1</a><a>2</a></a><b>3</b><b>4</b><c><x>5</x><x>6</x></c></root>");

    FlagVectorLabel f1;
    xpack::xml::decode(s, f1);
    EXPECT_EQ(f1.a.size(), 2U);
    EXPECT_EQ(f1.a[0], 1);
    EXPECT_EQ(f1.a[1], 2);
    EXPECT_EQ(f1.b.size(), 2U);
    EXPECT_EQ(f1.b[0], 3);
    EXPECT_EQ(f1.b[1], 4);
    EXPECT_EQ(f1.c.size(), 2U);
    EXPECT_EQ(f1.c[0], 5);
    EXPECT_EQ(f1.c[1], 6);
}

TEST(jsondata, memory) {
    xpack::JsonData *jd = new xpack::JsonData;

    xpack::json::decode("[0, 1, 2, 3]", *jd);
    EXPECT_TRUE(jd->IsArray());
    EXPECT_TRUE((*jd)[(size_t)0].IsNumber());
    EXPECT_EQ((*jd)[1].GetInt64(), 1);

    xpack::JsonData s = *jd;
    delete jd;

    EXPECT_TRUE(s.IsArray());
    EXPECT_TRUE(s[(size_t)0].IsNumber());
    EXPECT_EQ(s[1].GetInt64(), 1);    
}

// ++++++++++++++++++bug history+++++++++++++++++++++++
TEST(bughis, notexists) {
    Base b(9, "");
    xpack::json::decode("{\"a\":1}", b);
    EXPECT_EQ(b.a, 1);

    Base b1(10, "");
    xpack::xml::decode("<root><a>1</a></root>", b1);
    EXPECT_EQ(b1.a, 1);
}

struct OmitEmpty{
    int a;
    vector<int> b;
#ifdef X_PACK_SUPPORT_CXX0X
    shared_ptr<Base> c;
#else
    Base c;
#endif
    XPACK(X(F(OE), a, b, c));
};
TEST(bughis, omitempty) {
    OmitEmpty oe;
    oe.a = 0;
    oe.b.push_back(0);
    oe.b.push_back(0);
#ifdef X_PACK_SUPPORT_CXX0X
    oe.c.reset(new Base(0, ""));
#endif

    string s = xpack::json::encode(oe);
    EXPECT_EQ(s, "{\"b\":[0,0],\"c\":{\"a\":0,\"b\":\"\"}}");
    s = xpack::xml::encode(oe, "root");
    EXPECT_EQ(s, "<root><b><b>0</b><b>0</b></b><c><a>0</a><b/></c></root>");

    oe.b.clear();
    oe.a = 1;
    s = xpack::json::encode(oe);
    EXPECT_EQ(s, "{\"a\":1,\"c\":{\"a\":0,\"b\":\"\"}}");
    s = xpack::xml::encode(oe, "root");
    EXPECT_EQ(s, "<root><a>1</a><c><a>0</a><b/></c></root>");
}

#ifdef X_PACK_SUPPORT_CXX0X
struct SharedPtrNull{
    shared_ptr<xpack::JsonData> jd;
    XPACK(O(jd));
};
TEST(bughis, shared_ptr_null) {
    SharedPtrNull sn;

    xpack::json::decode("{\"jd\":null}", sn);
    EXPECT_TRUE(sn.jd);
    EXPECT_TRUE(sn.jd->IsNull());
    EXPECT_FALSE(sn.jd->IsArray());
}
#endif

int main(int argc, char *argv[]) {
#ifdef XGTEST
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#else
    (void)argc;
    (void)argv;
    TC_CONTAINER::RUN();
    return 0;
#endif
}
