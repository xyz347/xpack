#include <iostream>
#include <string>
#include "xpack/json.h"

using namespace std;

template<bool b> struct booltype {};

template<typename T, typename B = booltype<true> >
struct isxpack {static const bool value = false;};

template<typename T> 
struct isxpack<T, booltype<T::__x_pack_value> > {static const bool value = true; };


// without parents 
class MemberList {
public:
    std::vector<std::string> members;

    template<class T>
    bool encode(const char *key, const T&val, const xpack::Extend *ext) {
        if (NULL != key) {
            members.push_back(std::string(key));
        }
        return true;
    }
};

// with parents 
class MemberListP {
public:
    std::vector<std::string> members;
    

    // class without xpack
    template<class T>
    typename xpack::x_enable_if<!isxpack<T>::value&&!xpack::is_xpack_out<T>::value, bool>::type encode(const char *key, const T&val, const xpack::Extend *ext) {
        if (NULL != key) {
            members.push_back(std::string(key));
        }
        return true;
    }

    // class with xpack
    template<class T>
    typename xpack::x_enable_if<T::__x_pack_value && !xpack::is_xpack_out<T>::value, bool>::type encode(const char *key, const T&val, const xpack::Extend *ext) {
        if (NULL != key) {
            members.push_back(std::string(key));
        } else {
            val.__x_pack_encode(*this, val, ext);
        }
        return true;
    }

    // class with xpack_out
    template<class T>
    typename xpack::x_enable_if<xpack::is_xpack_out<T>::value, bool>::type encode(const char *key, const T&val, const xpack::Extend *ext) {
        if (NULL != key) {
            members.push_back(std::string(key));
        } else {
            __x_pack_encode_out(*this, val, ext);
        }
        return true;
    }
};

template <class T>
std::string GetMembers(const T&val) {
    MemberList m;
    val.__x_pack_encode(m, val, 0);

    return xpack::json::encode(m.members);
}

template <class T>
std::string GetMembersP(const T&val) {
    MemberListP m;
    val.__x_pack_encode(m, val, 0);

    return xpack::json::encode(m.members);
}

class Base1 {
public:
    string b1s;
    int b1i;
    XPACK(O(b1s, b1i));
};

struct Base2 {
    string b2s;
    int b2i;
};

XPACK_OUT(Base2, O(b2s, b2i));

struct N {
    string ns;
    int ni;
    XPACK(O(ns, ni));
};

struct Test:public Base1, Base2 {
    int a;
    int b;
    N   n;
    XPACK(I(Base1, Base2), O(a,b, n));
};


int main(int argc, char *argv[]) {
    Test t;
    std::cout<<GetMembers(t)<<std::endl;
    std::cout<<GetMembersP(t)<<std::endl;
    return 0;
}
