
#include <iostream>

#include "teststruct.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: ./exe <in json file>" << std::endl;
        return -1;
    }
    
    struct PublistToWeb ptw;
    xpack::json::decode_file(argv[1], ptw);
    std::cout << xpack::json::encode(ptw) << std::endl << std::endl;
    
    return 0;
}
