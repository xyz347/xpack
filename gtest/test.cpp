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
#endif

#include "xpack/json.h"
#include "xpack/xml.h"
#include "string.h"

using namespace std;

// BuiltInTypes
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
    XPACK(X(F(ATTR), sch, ch, uch, sh, ush), O(i, ui, l, ul, ll, ull, f, d, ld, b));
};

// simple struct used by other
struct Base {
    int     bi;
    string  bs;
    XPACK(O(bi, bs));
};

// test in another namespace
namespace otherns {

// test enum
enum Enum {
    E0 = 0,
    E1 = 1,
};

struct OtherNS:public Base {
    // test bit field
    short h:8;
    short l:8;
    Enum e;
};

}

// XPACK_OUT  must define in global namespace
XPACK_OUT(otherns::OtherNS, I(Base), B(F(0), h, l), E(F(0), e));

struct XTest :public otherns::OtherNS {
    string                  as1;    // alias name
    string                  as2;
    BuiltInTypes            types;
    vector<int>             vi;        // vector int
    vector<vector<int> >    vvi;    // vector vector int
    vector<string>          vs;        // vector string
    vector<vector<string> > vvs;    // vector vector string
    vector<Base>            vst;    // vector struct
    vector<vector<Base> >   vvst;    // vector vector struct

    set<int>                    si;
    list<int>                   li;
    map<string, int>            mi;
    map<string, Base>           mst;
#ifdef XGTEST
    unordered_map<string, Base> umst;
#else
    map<string, Base> umst;
#endif
    shared_ptr<Base>            spst;
    char                        charray[16];
#ifdef XPACK_SUPPORT_QT
    // Qt
    QString             qstr;
    QList<Base>         qlst;
    QVector<Base>       qvst;
    QMap<string, Base>  qmst;
    QMap<QString, Base> qmqsst;

    XPACK(I(otherns::OtherNS, Base), A(as1, "a1 json:alias1", as2, "a2 json:alias2"),
          O(types, vi, vvi, vs, vvs, vst, vvst),
          O(si,li,mi, mst, umst, spst, charray),
          O(qstr, qlst, qvst, qmst, qmqsst));
#else
    XPACK(I(otherns::OtherNS, Base), A(as1, "a1 json:alias1", as2, "a2 json:alias2"),
          O(types, vi, vvi, vs, vvs, vst, vvst),
          O(si,li,mi, mst, umst, spst, charray));
#endif
};

#ifdef XGTEST
void childeq(const XTest&cd) {
    EXPECT_EQ(cd.bi, 1024);
    EXPECT_EQ(cd.bs, "1024");

    EXPECT_EQ(cd.h, 10);
    EXPECT_EQ(cd.l, 24);
    EXPECT_EQ(cd.e, 1);

    EXPECT_EQ(cd.as1, "hello");
    EXPECT_EQ(cd.as2, "world");

    EXPECT_EQ(cd.vi.size(), 3);
    EXPECT_EQ(cd.vi[0], 1);
    EXPECT_EQ(cd.vi[1], 2);
    EXPECT_EQ(cd.vi[2], 4);

    EXPECT_EQ(cd.vvi.size(), 2);
    EXPECT_EQ(cd.vvi[0][0], 8);
    EXPECT_EQ(cd.vvi[0][1], 16);
    EXPECT_EQ(cd.vvi[0][2], 32);
    EXPECT_EQ(cd.vvi[1][0], 64);
    EXPECT_EQ(cd.vvi[1][1], 128);
    EXPECT_EQ(cd.vvi[1][2], 256);

    EXPECT_EQ(cd.vs.size(), 3);
    EXPECT_EQ(cd.vs[0], "hello");
    EXPECT_EQ(cd.vs[1], "hallo");
    EXPECT_EQ(cd.vs[2], "你好");

    EXPECT_EQ(cd.vvs.size(), 2);
    EXPECT_EQ(cd.vvs[0][0], "Python");
    EXPECT_EQ(cd.vvs[0][1], "Perl");
    EXPECT_EQ(cd.vvs[0][2], "Bash");
    EXPECT_EQ(cd.vvs[1][0], "C++");
    EXPECT_EQ(cd.vvs[1][1], "Golang");
    EXPECT_EQ(cd.vvs[1][2], "Rust");

    EXPECT_EQ(cd.vst.size(), 2);
    EXPECT_EQ(cd.vst[0].bi, 1);
    EXPECT_EQ(cd.vst[0].bs, "2");
    EXPECT_EQ(cd.vst[1].bi, 3);
    EXPECT_EQ(cd.vst[1].bs, "4");

    EXPECT_EQ(cd.vvst.size(), 2);
    EXPECT_EQ(cd.vvst[0][0].bi, 5);
    EXPECT_EQ(cd.vvst[0][0].bs, "6");
    EXPECT_EQ(cd.vvst[0][1].bi, 7);
    EXPECT_EQ(cd.vvst[0][1].bs, "8");
    EXPECT_EQ(cd.vvst[1][0].bi, 9);
    EXPECT_EQ(cd.vvst[1][0].bs, "10");

    EXPECT_EQ(cd.si.size(), 3);
    EXPECT_TRUE(cd.si.find(1)!=cd.si.end());
    EXPECT_TRUE(cd.si.find(3)!=cd.si.end());
    EXPECT_TRUE(cd.si.find(5)!=cd.si.end());

    auto siter = cd.li.begin();
    EXPECT_EQ(cd.li.size(), 3);
    EXPECT_EQ(*siter, 2); ++siter;
    EXPECT_EQ(*siter, 4); ++siter;
    EXPECT_EQ(*siter, 6); ++siter;

    EXPECT_EQ(cd.mi.find("a")->second, 1);
    EXPECT_EQ(cd.mi.find("b")->second, 2);
    EXPECT_EQ(cd.mi.find("c")->second, 3);

    EXPECT_EQ(cd.mst.find("d")->second.bi, 1);
    EXPECT_EQ(cd.mst.find("d")->second.bs, "2");
    EXPECT_EQ(cd.mst.find("e")->second.bi, 3);
    EXPECT_EQ(cd.mst.find("e")->second.bs, "4");

    EXPECT_EQ(cd.umst.find("f")->second.bi, 1);
    EXPECT_EQ(cd.umst.find("f")->second.bs, "2");
    EXPECT_EQ(cd.umst.find("g")->second.bi, 3);
    EXPECT_EQ(cd.umst.find("g")->second.bs, "4");

    EXPECT_EQ(cd.spst->bi, 10);
    EXPECT_EQ(cd.spst->bs, "24");

    EXPECT_TRUE(strcmp(cd.charray, "hello world")==0);

    EXPECT_EQ(cd.qstr, "1024");
#ifdef XPACK_SUPPORT_QT
    auto qlstiter = cd.qlst.begin();
    EXPECT_EQ(cd.qlst.size(), 2);
    EXPECT_EQ(qlstiter->bi, 1);
    EXPECT_EQ(qlstiter->bs, "2");++qlstiter;
    EXPECT_EQ(qlstiter->bi, 3);
    EXPECT_EQ(qlstiter->bs, "4");

    auto qvstiter = cd.qvst.begin();
    EXPECT_EQ(cd.qvst.size(), 2);
    EXPECT_EQ(qvstiter->bi, 5);
    EXPECT_EQ(qvstiter->bs, "6");++qvstiter;
    EXPECT_EQ(qvstiter->bi, 7);
    EXPECT_EQ(qvstiter->bs, "8");

    EXPECT_EQ(cd.qmst.find("d")->bi, 1);
    EXPECT_EQ(cd.qmst.find("d")->bs, "2");
    EXPECT_EQ(cd.qmst.find("e")->bi, 3);
    EXPECT_EQ(cd.qmst.find("e")->bs, "4");

    EXPECT_EQ(cd.qmqsst.find("e")->bi, 5);
    EXPECT_EQ(cd.qmqsst.find("e")->bs, "6");
    EXPECT_EQ(cd.qmqsst.find("f")->bi, 7);
    EXPECT_EQ(cd.qmqsst.find("f")->bs, "8");
#endif
    EXPECT_EQ(cd.types.sch, 48);
    EXPECT_EQ(cd.types.ch, 49);
    EXPECT_EQ(cd.types.uch, 50);
    EXPECT_EQ(cd.types.sh, 10);
    EXPECT_EQ(cd.types.ush, 24);
    EXPECT_EQ(cd.types.i, 10);
    EXPECT_EQ(cd.types.ui, 24);
    EXPECT_EQ(cd.types.l, 10);
    EXPECT_EQ(cd.types.ul, 24);
    EXPECT_EQ(cd.types.ll, 10);
    EXPECT_EQ(cd.types.ull, 24);
    EXPECT_FLOAT_EQ(cd.types.f, 2.718);
    EXPECT_DOUBLE_EQ(cd.types.d, 3.14);
    EXPECT_DOUBLE_EQ(cd.types.ld, 0.618);
    EXPECT_TRUE(cd.types.b);
}

TEST(json, testJson) {
    XTest cd;
    xpack::json::decode_file("test.json", cd);
    childeq(cd);
    string tjs = xpack::json::encode(cd);//, 0, 1, '\t');
    cout<<"json:"<<endl<<tjs<<endl;
    XTest cd1;
    xpack::json::decode(tjs, cd1);
    childeq(cd1);
}

TEST(xml, testXml) {
    XTest cd;
    xpack::xml::decode_file("test.xml", cd);
    childeq(cd);

    string str = xpack::xml::encode(cd, "root");//, 0, 1, '\t');
    XTest cd1;
    cout<<"xml:"<<endl<<str<<endl;
    xpack::xml::decode(str, cd1);
    childeq(cd1);
}
#endif

int main(int argc, char *argv[]) {
#ifdef XGTEST
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#else
    XTest j1;
    XTest j2;
    string s1;
    string s2;

    cout<<"test json....";
    xpack::json::decode_file("test.json", j1);
    s1 = xpack::json::encode(j1);
    xpack::json::decode(s1, j2);
    s2 = xpack::json::encode(j2);
    if (0 != s1.compare(s2)) {
        cout<<"fail(json not same)"<<endl<<"json1:"<<endl<<s1<<endl<<"json2:"<<endl<<s2<<endl;
    } else {
        cout<<"done"<<endl;
    }

    XTest x1;
    XTest x2;
    cout<<"test xml....";
    xpack::xml::decode_file("test.xml", x1);
    s1 = xpack::xml::encode(x1, "root");
    xpack::xml::decode(s1, x2);
    s2 = xpack::xml::encode(x2, "root");
    if (0 != s1.compare(s2)) {
        cout<<"fail(xml not same)"<<endl<<"xml1:"<<endl<<s1<<endl<<"xml2:"<<endl<<s2<<endl;
    } else {
        cout<<"done"<<endl;
    }
    return 0;
#endif
}
