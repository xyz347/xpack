#include <iostream>
#include <string>
#include "xpack/json.h"

using namespace std;


// assign value by name
class AssignDecoder {
public:
    AssignDecoder(const std::vector<std::string>&vs, const unsigned char *src, unsigned char *dst) {
        for (size_t i=0; i<vs.size(); ++i) {
            fields.insert(vs[i]);
        }
        this->src = src;
        this->dst = dst;
    }

    // decoder need implemetion decode function
    template<class T>
    bool decode(const char *key, T&val, const xpack::Extend *ext) {
        (void)ext;
        // set value by offset
        if (fields.end() != fields.find(key)) {
            unsigned int offset = (unsigned int)((unsigned char*)&val - dst);
            val = *((T*)(src+offset));
        }
        return true;
    }
private:
    std::set<std::string> fields;
    const unsigned char *src;
    unsigned char *dst;
};

// SetFields<T>(src, dst, "a,b") means dst.a = src.a; dst.b = src.b;
template <class T>
void SetFields(const T&src, T&dst, const std::string&fields) {
    std::vector<std::string> vs;
    xpack::Util::split(vs, fields, ',');
    if (vs.size() == 0) {
        return;
    }

    AssignDecoder de(vs, (unsigned char*)(&src), (unsigned char*)(&dst));
    dst.__x_pack_decode(de, dst, NULL);
}

struct Data {
    int a;
    int b;
    std::string c;
    XPACK(O(a,b,c));
};


int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    Data d1;
    Data d2;
    d1.a = 5;
    d1.b = 10;
    d1.c = "hello";
    d2.b = 9;
    SetFields<Data>(d1, d2, "a,c");
    cout<<d2.a<<','<<d2.b<<','<<d2.c<<endl;
    return 0;
}
