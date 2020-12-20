/*
* Copyright (C) 2021 Duowan Inc. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#ifndef __X_PACK_TRAITS_H
#define __X_PACK_TRAITS_H

#if __GXX_EXPERIMENTAL_CXX0X__ || _MSC_VER>=1700
#define X_PACK_SUPPORT_CXX0X 1
#endif

namespace xpack {

// implement std::enable_if
template<bool B, class T = void>
struct x_enable_if {};
template <class T>
struct x_enable_if<true, T> { typedef T type; };

// mark XPACK_OUT
template <class T>
struct is_xpack_out{static bool const value = false;};

// for bitfield, declare raw type. thx https://stackoverflow.com/a/12199635/5845104
template<int N> struct x_size { char value[N]; };
x_size<1> x_decltype_encode(char);
x_size<2> x_decltype_encode(signed char);
x_size<3> x_decltype_encode(unsigned char);
x_size<4> x_decltype_encode(short);
x_size<5> x_decltype_encode(unsigned short);
x_size<6> x_decltype_encode(int);
x_size<7> x_decltype_encode(unsigned int);
x_size<8> x_decltype_encode(long);
x_size<9> x_decltype_encode(unsigned long);
x_size<10> x_decltype_encode(long long);
x_size<11> x_decltype_encode(unsigned long long);

template<int N> struct x_decltype_decode {};
template <> struct x_decltype_decode<1> {typedef char type;};
template <> struct x_decltype_decode<2> {typedef signed char type;};
template <> struct x_decltype_decode<3> {typedef unsigned char type;};
template <> struct x_decltype_decode<4> {typedef short type;};
template <> struct x_decltype_decode<5> {typedef unsigned short type;};
template <> struct x_decltype_decode<6> {typedef int type;};
template <> struct x_decltype_decode<7> {typedef unsigned int type;};
template <> struct x_decltype_decode<8> {typedef long type;};
template <> struct x_decltype_decode<9> {typedef unsigned long type;};
template <> struct x_decltype_decode<10> {typedef long long type;};
template <> struct x_decltype_decode<11> {typedef unsigned long long type;};

}

#define x_pack_decltype(T) typename xpack::x_decltype_decode<sizeof(xpack::x_decltype_encode(T))>::type

#endif

