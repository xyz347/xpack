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

#ifndef __X_PACK_NUMERIC_H
#define __X_PACK_NUMERIC_H

namespace xpack {

template <class T>
struct numeric{static bool const value = false; static bool const is_integer = false;static bool const is_float = false;};

template<>
struct numeric<char>{static bool const value = true; static bool const is_integer = true;};

template<>
struct numeric<signed char>{static bool const value = true; static bool const is_integer = true;};

template<>
struct numeric<unsigned char>{static bool const value = true; static bool const is_integer = true;};

template<>
struct numeric<short>{static bool const value = true; static bool const is_integer = true;};

template<>
struct numeric<unsigned short>{static bool const value = true; static bool const is_integer = true;};

template<>
struct numeric<int>{static bool const value = true; static bool const is_integer = true;};

template<>
struct numeric<unsigned int>{static bool const value = true; static bool const is_integer = true;};

template<>
struct numeric<long>{static bool const value = true; static bool const is_integer = true;};

template<>
struct numeric<unsigned long>{static bool const value = true; static bool const is_integer = true;};

template<>
struct numeric<long long>{static bool const value = true; static bool const is_integer = true;};

template<>
struct numeric<unsigned long long>{static bool const value = true; static bool const is_integer = true;};

template<>
struct numeric<float>{static bool const value = true;static bool const is_float = true;};

template<>
struct numeric<double>{static bool const value = true;static bool const is_float = true;};

template<>
struct numeric<long double>{static bool const value = true;static bool const is_float = true;};

}

#endif
