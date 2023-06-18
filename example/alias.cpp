#include <iostream>
#include "xpack/json.h"
#include "xpack/xml.h"

using namespace std;

struct Test {
    long uid;
    string  name;
    XPACK(A(uid, "id xml:xid"), O(name)); // xml use 'xid' and others use 'id'
};

int main() {
    Test t;
    string json="{\"id\":123, \"name\":\"Pony\"}";

    xpack::json::decode(json, t);
    cout<<t.uid<<endl;

    string xstr = xpack::xml::encode(t, "root");
    cout<<xstr<<endl;
    Test t1;
    xpack::xml::decode(xstr, t1);
    cout<<t1.uid<<endl;

    return 0;
}
