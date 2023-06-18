#include <iostream>
#include "xpack/json.h"
#include "xpack/xml.h"

using namespace std;

struct Test {
    short ver:8;
    short len:8;
    string  name;
    XPACK(B(F(0), ver, len), O(name));
};

int main() {
    Test t;
    string json="{\"ver\":4, \"len\":20, \"name\":\"IPv4\"}";

    xpack::json::decode(json, t);
    cout<<t.ver<<endl;
    cout<<t.len<<endl;
    cout<<xpack::xml::encode(t, "root")<<endl;
    return 0;
}
