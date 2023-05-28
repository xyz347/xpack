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

// Linux only, need gtest

#include <iostream>
#ifdef XGTEST
#include<gtest/gtest.h>
#else
#include "gtest_stub.h"
#endif

#include "xpack/json.h"
#include "xpack/bson.h"
#include "string.h"

using namespace std;

struct Base {
    int    a;
    string b;
    Base(const int _a=0, const string&_b=""):a(_a), b(_b){}
    XPACK(O(a, b));
};

struct POD {
    int  a;
    char b[10];
    XPACK(O(a, b));
};

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
    XPACK(AF(F(ATTR), sch, "xml:s:ch"), X(F(ATTR), ch, uch, sh, ush), O(i, ui, l, ul, ll, ull, f, d, ld, b));
};


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

    xpack::BsonEncoder en;
    string js = en.encode_as_json(bt);
    cout<<"bson:"<<js<<endl;

    BuiltInTypes j1;
    xpack::bson::decode(en.encode(bt), j1);
    checkBuiltInTypes(j1, bt);
}

// ++++++++++++++++++++bit field+++++++++++++++++++++++++++
struct BitField {
    short a:8;
    short b:8;

    XPACK(B(F(0), a, b));
};

void checkBitField(const BitField&a, const BitField&b) {
    EXPECT_EQ(a.a, b.a);
    EXPECT_EQ(a.b, b.b);
}
TEST(bitfiled, base) {
    BitField bf;
    bf.a = 0x70;
    bf.b = 0x7f;

    xpack::BsonEncoder en;
    string js = en.encode_as_json(bf);

    BitField j1;
    xpack::bson::decode(en.encode(bf), j1);
    checkBitField(j1, bf);
}

// +++++++++++++++++++ enum +++++++++++++++++++++
enum Enum1 {
    E0 = 0,
    E1 = 1,
};
enum class Enum2:int64_t {
    E2 = 2,
    E3 = 3,
};
struct EnumBase {
    Enum1 e1;
    Enum2 e2;

    XPACK(O(e1), E(F(0), e2));
};

void checkEnumBase(const EnumBase&a, const EnumBase&b) {
    EXPECT_EQ(a.e1, b.e1);
    EXPECT_EQ(a.e2, b.e2);
}
TEST(enum, base) {
    EnumBase eb;
    eb.e1 = E0;
    eb.e2 = Enum2::E3;

    xpack::BsonEncoder en;
    string js = en.encode_as_json(eb);

    EnumBase j1;
    xpack::bson::decode(en.encode(eb), j1);
    checkEnumBase(j1, eb);
}

// ++++++++++++++++inherit+++++++++++++++++++++++++++++
struct InheritBase {
    int b1;
    string b2;

    XPACK(O(b1, b2));
};

struct InheritChild:public InheritBase {
    int c1;
    string c2;

    XPACK(I(InheritBase), O(c1, c2));
};

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

    xpack::BsonEncoder en;
    string js = en.encode_as_json(c);

    InheritChild j1;
    xpack::bson::decode(en.encode(c), j1);
    checkInherit(j1, c);
}

// ++++++++++++++++++++++++array++++++++++++++++++++++++++
struct Array {
    int  a[3];
    char b[3];
    Base c[3];

    XPACK(O(a, b, c));
};

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

    xpack::BsonEncoder en;
    string js = en.encode_as_json(a);

    Array j1;
    xpack::bson::decode(en.encode(a), j1);
    checkArray(j1, a);
}


// +++++++++++++++ container ++++++++++++++++++++++
struct ContainerBase {
    map<string, int> m;
    set<int> s;
    vector<int> v;
    list<int> l;

    vector<vector<int> > vv;

    XPACK(O(m, s, v, l, vv));
};

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

    xpack::BsonEncoder en;
    string js = en.encode_as_json(cb);

    ContainerBase j1;
    xpack::bson::decode(en.encode(cb), j1);
    checkContainerBase(j1);
}
struct ContainerStruct {
    map<string, Base> m;
    vector<Base> v;
    list<Base> l;

    vector<vector<Base> > vv;

    XPACK(O(m, v, l, vv));
};

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

    xpack::BsonEncoder en;
    string js = en.encode_as_json(cb);

    ContainerStruct j1;
    xpack::bson::decode(en.encode(cb), j1);
    checkContainerStruct(j1);
}

// ++++++++++++++++++ BSON TYPES ++++++++++++++++++++++++++++
struct BsonTypes {
    bson_date_time_t dt;
    bson_timestamp_t ts;
    bson_binary_t    binary;
    bson_regex_t     regex;

    bson_oid_t       oid;
    bson_decimal128_t decimal;
    XPACK(A(oid, "_id"), O(dt, ts, binary, regex, decimal));
};

void checkBsonTypes(const BsonTypes&a, const BsonTypes&b, bool checkDecimal) {
    EXPECT_EQ(a.dt.ts, b.dt.ts);

    EXPECT_EQ(a.ts.timestamp, b.ts.timestamp);
    EXPECT_EQ(a.ts.increment, b.ts.increment);

    EXPECT_EQ(a.binary.data, b.binary.data);
    EXPECT_EQ(a.binary.subType, b.binary.subType);

    EXPECT_EQ(a.regex.pattern, b.regex.pattern);
    EXPECT_EQ(a.regex.options, b.regex.options);

    EXPECT_EQ(0, memcmp(a.oid.bytes, b.oid.bytes, sizeof(a.oid.bytes)));

    if (checkDecimal) {
        EXPECT_EQ(a.decimal.high, b.decimal.high);
        EXPECT_EQ(a.decimal.low, b.decimal.low);
    }
}
TEST(base, bson_types) {
    BsonTypes bt;

    bt.dt.ts = 1638374400018;

    bt.ts.timestamp = 1638374400;
    bt.ts.increment = 1;

    bt.binary.data = "hello";
    bt.binary.subType = BSON_SUBTYPE_USER;

    bt.regex.pattern = "^([0-9a-zA-Z]+)";
    bt.regex.options = "i";
    bt.regex.ver = 2;

    uint8_t bytes[] = {0x57, 0xe1, 0x93, 0xd7, 0xa9, 0xcc, 0x81, 0xb4, 0x02, 0x74, 0x98, 0xb5};
    memcpy((void*)bt.oid.bytes, bytes, sizeof(bt.oid.bytes));

    bt.decimal.high = 1;
    bt.decimal.low = 2;

    string js = xpack::json::encode(bt);
    cout<<"json:"<<js<<endl;
    BsonTypes bt1;
    xpack::json::decode(js, bt1);
    checkBsonTypes(bt, bt1, false);

    xpack::BsonEncoder en;
    string js1 = en.encode_as_json(bt);
    cout<<"bjson:"<<js1<<endl;
    BsonTypes bt2;
    xpack::json::decode(js1, bt2);
    checkBsonTypes(bt, bt2, false);

    BsonTypes bt3;
    xpack::bson::decode(en.encode(bt), bt3);
    checkBsonTypes(bt, bt3, true);
}

TEST(bson, builders) {
    static xpack::BsonBuilder bd("{?:?, 'users':?}");
    EXPECT_TRUE(bd.Error().empty());

    vector<int> v(3);
    v[0] = 1; v[1] = 2; v[2] = 3;
    cout<<"json1:"<<bd.EncodeAsJson("hi", true, v)<<endl;
    cout<<"json2:"<<bd.EncodeAsJson("uid", 123.0, "LiLei/HanMeimei/Jim")<<endl;
    cout<<"json3:"<<bd.EncodeAsJson("Lang", "C++", "")<<endl;
}

// +++++++++++++++++++ QT ++++++++++++++++++++
#ifdef XPACK_SUPPORT_QT
struct ContainerQT {
    QMap<string, Base> m;
    QVector<Base> v;
    QList<Base> l;

    QVector<QVector<Base> > vv;

    XPACK(O(m, v, l, vv));
};

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

    xpack::BsonEncoder en;
    en.encode(NULL, cb, NULL);
    string js = en.Json();

    ContainerQT j1;
    xpack::bson::decode(en.String(), j1);
    checkContainerQT(j1);
}
#endif


// +++++++++++++++++ flags ++++++++++++++++++
struct FlagM {
    int a;
    string b;
    XPACK(O(a), M(b));
};
TEST(flags, M) {
    static xpack::BsonBuilder bd("{'a':1}");

    bool except = false;
    try {
        FlagM j1;
        xpack::bson::decode(bd.Encode(), j1);
    } catch(...) {
        except = true;
    }
    EXPECT_TRUE(except);
}
// ++++++++++++++++++bug history+++++++++++++++++++++++
TEST(bughis, notexists) {
    Base b(9, "");

    static xpack::BsonBuilder bd("{'a':1}");
    xpack::bson::decode(bd.Encode(), b);
    EXPECT_EQ(b.a, 1);
}

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
