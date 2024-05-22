#include <iostream>
#include <xpack/xml.h>

using namespace std;

struct Base {
    int a;
    int b;
    string c;
    XPACK(X(F(ATTR), a, b), X(F(XML_CONTENT), c));
};

struct Root {
    Base root;
    XPACK(O(root));
};


int main() {
    Root r;
    r.root.a = 1;
    r.root.b = 2;
    r.root.c = "hello";
    string s = xpack::xml::encode(r, "root", 0, 4, ' ');
    cout<<s<<endl;
    string s1 = xpack::xml::encode(r, "root");
    cout<<s1<<endl;

    Root r1,r2;
    xpack::xml::decode(s, r1);
    xpack::xml::decode(s1, r2);

    cout<<xpack::xml::encode(r1, "r1")<<endl;
    cout<<xpack::xml::encode(r2, "r2")<<endl;

    return 0;
}
