#include <iostream>
#include "xpack/json.h"

class MemberList {
public:
    std::vector<std::string> members;

    template<class T>
    bool encode(const char *key, const T&val, const xpack::Extend *ext) {
        members.push_back(std::string(key));
        return true;
    }
};

template <class T>
std::string GetMembers(const T&val) {
    MemberList m;
    val.__x_pack_encode(m, val, 0);

    return xpack::json::encode(m.members);
}


struct Test {
    int a;
    int b;
    XPACK(O(a,b));
};

int main(int argc, char *argv[]) {
    Test t;
    std::cout<<GetMembers(t)<<std::endl;
    return 0;
}
