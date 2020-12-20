/*
* Copyright (C) 2021 Duowan Inc. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#ifndef __X_PACK_H
#define __X_PACK_H

#include "extend.h"
#include "l1l2_expand.h"


/*
    work with X_PACK_N to expand macro
    struct MyX {
        int         a;
        std::string b;
        double      c;
        XPACK(A(a,"_id"), O(b,c));
    };

    macro expand order:
        XPACK(A(a,"_id"), O(b,c))
    --> X_PACK_N(X_PACK_L1, X_PACK_L1_DECODE, , A(a, "_id"), O(b,c))
    --> X_PACK_L1_2(X_PACK_L1_DECODE, A(a, "_id"), O(b,c))
    --> X_PACK_L1_DECODE(A(a, "_id")) X_PACK_L1_DECODE(O(b,c))
    --> X_PACK_L1_DECODE_A(a, "_id") X_PACK_L1_DECODE_O(b,c)
    --> X_PACK_ACT_DECODE_A(a, "_id") X_PACK_N2(X_PACK_L2, X_PACK_ACT_DECODE_O, b, c) // https://gcc.gnu.org/onlinedocs/cpp/Self-Referential-Macros.html  so we need define X_PACK_N2. if use X_PACK_N preprocessor will treat is as Self-Referential-Macros
    --> X_PACK_ACT_DECODE_A(a, "_id") X_PACK_L2_2(X_PACK_ACT_DECODE_O, b, c)
    --> X_PACK_ACT_DECODE_A(a, "_id") X_PACK_ACT_DECODE_O(b) X_PACK_ACT_DECODE_O(c)
    --> // expand to convert code
*/

// flag only work for this member
#define X_EXPAND_FLAG_F(...)    int __x_pack_flag = 0 X_PACK_N2(X_PACK_L2, X_PACK_ACT_FLAG, __VA_ARGS__) ;
#define X_PACK_ACT_FLAG(F)        | X_PACK_FLAG_##F

/*
  X(F(x,y,z), member1, member2, ....)
  O  same as X(F(0), member1, member2, ...)
  M  same as X(F(M), member1, member2, ...)
  A  Alias. A(member1, alias1, member2, alias2, ...)
  AF Alias with Flag. A(F(x,y,z), member1, alias1, member2, alias2 ...)
  I  Inherit
  B  bitfield
*/

/////////////////////////// XPACK /////////////////////////////
//=======DECODE
#define X_PACK_L1_DECODE(x)             { X_PACK_L1_DECODE_##x }
//----
#define X_PACK_L1_DECODE_X(FLAG, ...)   X_EXPAND_FLAG_##FLAG xpack::Extend __x_pack_ext(__x_pack_flag, NULL); X_PACK_N2(X_PACK_L2, X_PACK_DECODE_ACT_O, __VA_ARGS__)
#define X_PACK_L1_DECODE_E(FLAG, ...)   X_EXPAND_FLAG_##FLAG xpack::Extend __x_pack_ext(__x_pack_flag, NULL); X_PACK_N2(X_PACK_L2, X_PACK_DECODE_ACT_E, __VA_ARGS__)
#define X_PACK_L1_DECODE_B(FLAG, ...)   X_EXPAND_FLAG_##FLAG xpack::Extend __x_pack_ext(__x_pack_flag, NULL); X_PACK_N2(X_PACK_L2, X_PACK_DECODE_ACT_B, __VA_ARGS__)
#define X_PACK_L1_DECODE_AF(FLAG, ...)  X_EXPAND_FLAG_##FLAG X_PACK_N2(X_PACK_L2_2, X_PACK_DECODE_ACT_A, __VA_ARGS__) // extend define in ACTION

#define X_PACK_L1_DECODE_O(...)         X_PACK_L1_DECODE_X(F(0), __VA_ARGS__)
#define X_PACK_L1_DECODE_M(...)         X_PACK_L1_DECODE_X(F(M), __VA_ARGS__)
#define X_PACK_L1_DECODE_A(...)         X_PACK_L1_DECODE_AF(F(0), __VA_ARGS__)

#define X_PACK_L1_DECODE_I(...)         X_PACK_N2(X_PACK_L2, X_PACK_DECODE_ACT_I, __VA_ARGS__)
//=======ENCODE
#define X_PACK_L1_ENCODE(x) { X_PACK_L1_ENCODE_##x }
//-----
#define X_PACK_L1_ENCODE_X(FLAG, ...)   X_EXPAND_FLAG_##FLAG xpack::Extend __x_pack_ext(__x_pack_flag, NULL); X_PACK_N2(X_PACK_L2, X_PACK_ENCODE_ACT_O, __VA_ARGS__)
#define X_PACK_L1_ENCODE_E(FLAG, ...)   X_EXPAND_FLAG_##FLAG xpack::Extend __x_pack_ext(__x_pack_flag, NULL); X_PACK_N2(X_PACK_L2, X_PACK_ENCODE_ACT_E, __VA_ARGS__)
#define X_PACK_L1_ENCODE_B(FLAG, ...)   X_EXPAND_FLAG_##FLAG xpack::Extend __x_pack_ext(__x_pack_flag, NULL); X_PACK_N2(X_PACK_L2, X_PACK_ENCODE_ACT_B, __VA_ARGS__)
#define X_PACK_L1_ENCODE_AF(FLAG, ...)  X_EXPAND_FLAG_##FLAG X_PACK_N2(X_PACK_L2_2, X_PACK_ENCODE_ACT_A, __VA_ARGS__) // extend define in ACTION

#define X_PACK_L1_ENCODE_O(...)         X_PACK_L1_ENCODE_X(F(0), __VA_ARGS__)
#define X_PACK_L1_ENCODE_M(...)         X_PACK_L1_ENCODE_X(F(M), __VA_ARGS__)
#define X_PACK_L1_ENCODE_A(...)         X_PACK_L1_ENCODE_AF(F(0), __VA_ARGS__)
//-----
#define X_PACK_L1_ENCODE_I(...)         X_PACK_N2(X_PACK_L2, X_PACK_ENCODE_ACT_I, __VA_ARGS__)


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ decode act ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define X_PACK_DECODE_ACT_O(M)      \
        __x_pack_obj.decode(#M, __x_pack_self.M, &__x_pack_ext);

// enum for not support c++11
#define X_PACK_DECODE_ACT_E(M)      \
        __x_pack_obj.decode(#M, *((int*)&__x_pack_self.M), &__x_pack_ext);


#define X_PACK_DECODE_ACT_A(M, NAME)                                      \
    {                                                                     \
        static xpack::Alias __x_pack_alias(#M, NAME);                     \
        xpack::Extend __x_pack_ext(__x_pack_flag, &__x_pack_alias);       \
        const char *__new_name = __x_pack_alias.Name(__x_pack_obj.Type());\
        __x_pack_obj.decode(__new_name, __x_pack_self.M, &__x_pack_ext);  \
    }

// Inheritance B::__x_pack_decode(__x_pack_obj)
#define X_PACK_DECODE_ACT_I(P)   __x_pack_obj.decode(NULL, static_cast<P&>(__x_pack_self), NULL);

// bitfield, not support alias
#define X_PACK_DECODE_ACT_B(B)                                \
    {                                                         \
        x_pack_decltype(__x_pack_self.B) __x_pack_tmp;        \
        __x_pack_obj.decode(#B, __x_pack_tmp, &__x_pack_ext); \
        __x_pack_self.B = __x_pack_tmp;\
    }

// ~~~~~~~~~~~~~~~~~~~~~~~ encode act ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define X_PACK_ENCODE_ACT_O(M)      \
        __x_pack_obj.encode(#M, __x_pack_self.M, &__x_pack_ext);

#define X_PACK_ENCODE_ACT_E(M)      \
        __x_pack_obj.encode(#M, (const int&)__x_pack_self.M, &__x_pack_ext);

#define X_PACK_ENCODE_ACT_A(M, NAME)                                      \
    {                                                                     \
        static xpack::Alias __x_pack_alias(#M, NAME);                     \
        xpack::Extend __x_pack_ext(__x_pack_flag, &__x_pack_alias);       \
        const char *__new_name = __x_pack_alias.Name(__x_pack_obj.Type());\
        __x_pack_obj.encode(__new_name, __x_pack_self.M, &__x_pack_ext);  \
    }

#define X_PACK_ENCODE_ACT_B(M)      \
        __x_pack_obj.encode(#M, __x_pack_self.M, &__x_pack_ext);

#define X_PACK_ENCODE_ACT_I(P)                                                                             \
        {                                                                                                  \
            xpack::Extend __x_pack_tmp_ext(0,NULL); __x_pack_tmp_ext.ctrl_flag |= X_PACK_CTRL_FLAG_INHERIT;\
            __x_pack_obj.encode(NULL, static_cast<const P&>(__x_pack_self), &__x_pack_tmp_ext);            \
        }


// for mark defined XPACK
#define X_PACK_COMMON \
public:               \
    static bool const __x_pack_value = true;

// decode function
#define X_PACK_DECODE_BEGIN                         \
    template<class __X_PACK_DOC, class __X_PACK_ME> \
    void __x_pack_decode(__X_PACK_DOC& __x_pack_obj, __X_PACK_ME &__x_pack_self, const xpack::Extend *__x_pack_extp) {

// encode function
#define X_PACK_ENCODE_BEGIN                          \
    template <class __X_PACK_DOC, class __X_PACK_ME> \
    void __x_pack_encode(__X_PACK_DOC& __x_pack_obj, const __X_PACK_ME &__x_pack_self, const xpack::Extend *__x_pack_extp) const {


// out decode function
#define X_PACK_DECODE_BEGIN_OUT(NAME) \
    template<typename __X_PACK_DOC>   \
    void __x_pack_decode_out(__X_PACK_DOC& __x_pack_obj, NAME & __x_pack_self, const xpack::Extend *__x_pack_extp) {

// out encode function
#define X_PACK_ENCODE_BEGIN_OUT(NAME)  \
    template <class __X_PACK_DOC>      \
    void __x_pack_encode_out(__X_PACK_DOC& __x_pack_obj, const NAME &__x_pack_self, const xpack::Extend *__x_pack_extp) {


#define XPACK(...)   \
    X_PACK_COMMON    \
    X_PACK_DECODE_BEGIN X_PACK_N(X_PACK_L1, X_PACK_L1_DECODE, __VA_ARGS__) }  \
    X_PACK_ENCODE_BEGIN X_PACK_N(X_PACK_L1, X_PACK_L1_ENCODE, __VA_ARGS__) }

#define XPACK_OUT(NAME, ...)   \
namespace xpack {              \
    template<> struct is_xpack_out<NAME> {static bool const value = true;}; \
    X_PACK_DECODE_BEGIN_OUT(NAME) X_PACK_N(X_PACK_L1, X_PACK_L1_DECODE, __VA_ARGS__) }  \
    X_PACK_ENCODE_BEGIN_OUT(NAME) X_PACK_N(X_PACK_L1, X_PACK_L1_ENCODE, __VA_ARGS__) }  \
}

#endif

