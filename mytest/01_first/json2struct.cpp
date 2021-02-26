
#include <iostream>

#include "caddata.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: ./exe <in json file>" << std::endl;
        return -1;
    }
    
    struct PublistToWeb ptw;
    x2struct::X::loadjson(argv[1], ptw, true);
    std::cout << x2struct::X::tojson(ptw) << std::endl << std::endl;
    
    return 0;
}
