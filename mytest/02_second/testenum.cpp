
#include <iostream>

#include "xpack/json.h"

enum Enum {
    X = 0,
    Y = 1,
    Z = 2,
};

struct Test {
    std::string  name;
    Enum    e;
    XPACK(O(name), E(F(0), e));
};

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    Test t;
    std::string json="{\"name\":\"IPv4\", \"e\":1}";

    xpack::json::decode(json, t);
    std::cout << t.name << std::endl;
    std::cout << t.e << std::endl;
    
    return 0;
}
