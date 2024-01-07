#include <iostream>
#include <xpack/xml.h>

using namespace std;

struct Base {
    int a;
    int b;
    XPACK(O(a, b));//, X(F(ATTR), b));
};

struct Root {
    Base root;
    XPACK(O(root));
};

struct Top {
    int k;
    string n;
    vector<vector<Base> > bs;
    vector<int> vi;
    float f;
    std::string name; // cdata
    XPACK(O(k, n, f), A(bs, "xml:base,sbs", vi, "xml:vi,vl@x", name, "xml:name,cdata"));
};

int main() {
    Top t;
    t.k = 666;
    t.n = "你好";
    t.f = 1.23333;
    t.vi.resize(2);
    t.vi[0] = 1;
    t.vi[1] = 2;
    t.bs.resize(2);
    t.bs[0].resize(2);
    t.bs[1].resize(2);
    t.bs[0][0].a = 100;
    t.bs[0][0].b = 200;
    t.bs[0][1].a = 101;
    t.bs[0][1].b = 201;
    t.bs[1][0].a = 110;
    t.bs[1][0].b = 210;
    t.bs[1][1].a = 111;
    t.bs[1][1].b = 211;
    t.name = "好好学习，天天向上<good good study, day day up>";
    string s = xpack::xml::encode(t, "root", 0, 4, ' ');
    cout<<s<<endl;

    xpack::XmlEncoder en;
    en.SetMaxDecimalPlaces(3);
    cout<<"=========="<<endl<<en.encode(t, "root")<<endl;

    Top t1;
    xpack::xml::decode(s, t1);
    cout<<xpack::xml::encode(t1, "root", 0, 4, ' ')<<endl;

    Base b1;
    bool ret = xpack::XmlDecoder().decode("<root><a>1</a><b>2</b></root>", b1);
    cout<<ret<<','<<b1.a<<','<<b1.b<<endl;

    Root r1;
    ret = xpack::XmlDecoder().decode("<root><a>1</a><b>2</b></root>", r1, true);
    cout<<ret<<','<<r1.root.a<<','<<r1.root.b<<endl;

    return 0;
}
