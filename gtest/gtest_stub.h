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



#ifndef __X_GTEST_STUB_H
#define __X_GTEST_STUB_H

#include <vector>
#include <iostream>

#ifdef XPACK_SUPPORT_QT
#include <QString>
#endif

#include <xpack/traits.h>
#include <xpack/numeric.h>

// test callback function
typedef void(*test_case)();

// a test case
struct text_ctx {
    const char *group;
    const char *name;
    test_case tc;
};

class Status {
public:
    static int&c() {
        static int _c;
        return _c;
    }
};

// to collect all test
class TC_CONTAINER {
public:
    // add a test case to container
    static void add(test_case tc, const char* g, const char*n) {
        text_ctx tx;
        tx.group = g;
        tx.name = n;
        tx.tc = tc;
        I()._tcs.push_back(tx);
    }

    // run all test case
    static void RUN() {
        const std::vector<text_ctx>& tcs = I()._tcs;
        for (size_t i=0; i<tcs.size(); ++i) {
            Status::c() = 0;
            std::cout<<tcs[i].group<<" "<<tcs[i].name<<" start --->"<<std::endl;
            tcs[i].tc();
            if (Status::c() == 0) {
                std::cout<<tcs[i].group<<" "<<tcs[i].name<<" passed.";
            } else {
                std::cout<<tcs[i].group<<" "<<tcs[i].name<<" fail. ";
            }
            std::cout<<"<---"<<std::endl;
        }
    }
private:
    // singleton
    static TC_CONTAINER& I() {
        static TC_CONTAINER _inst;
        return _inst;
    }

    std::vector<text_ctx> _tcs;
};

// add test case in ctor
class AUTO_ADD_TC {
public:
    AUTO_ADD_TC(test_case tc, const char* g, const char*n) {
        TC_CONTAINER::add(tc, g, n);
    }
};


// type convert to support std::cout
class TV {
public:
#ifdef X_PACK_SUPPORT_CXX0X
    template <class TYPE>
    static typename std::enable_if<std::is_enum<TYPE>::value, int64_t>::type tv(const TYPE &d) {
        return (int64_t)d;
    }
    template <class TYPE>
    static typename std::enable_if<!std::is_enum<TYPE>::value, TYPE>::type tv(const TYPE &d) {
        return d;
    }
    static const char* tv(const char* d) {
        return d;
    }
#else
    template <class TYPE>
    static const TYPE& tv(const TYPE&d) {
        return d;
    }
#endif

    #ifdef XPACK_SUPPORT_QT
    static std::string tv(const QString &d) {
        return d.toStdString();
    }
    #endif
};

#define expect_eq_float(a,b,delta) \
do {\
    if (!((a)>=(b)-delta && (a)<=(b)+delta)) {\
        std::cout<<std::endl<<"++++++++++"<<__FILE__<<':'<<__LINE__<<'['<<__FUNCTION__<<']'<<"EXPECT_EQ fail.++++++++"<<std::endl;\
        std::cout<<"expect:"<<TV::tv(b)<<std::endl;\
        std::cout<<"actual:"<<TV::tv(a)<<std::endl<<std::endl;\
        ++Status::c();\
    }\
}while(false)

#define EXPECT_EQ(a,b) \
do {\
    if (!((a) == (b))) {\
        std::cout<<std::endl<<"++++++++++"<<__FILE__<<':'<<__LINE__<<'['<<__FUNCTION__<<']'<<"EXPECT_EQ fail.++++++++"<<std::endl;\
        std::cout<<"expect:"<<TV::tv(b)<<std::endl;\
        std::cout<<"actual:"<<TV::tv(a)<<std::endl<<std::endl;\
        ++Status::c();\
    }\
}while(false)

#define TEST(a, b) static void a##_##b();  static AUTO_ADD_TC __aat__##a##_##b(a##_##b, #a, #b); static void a##_##b()

#define EXPECT_FLOAT_EQ(a, b) expect_eq_float(a, b, 0.000001)
#define EXPECT_DOUBLE_EQ(a, b) expect_eq_float(a, b, 0.000001)

#define EXPECT_TRUE(a) \
do {\
    if (!(a)) {\
        std::cout<<std::endl<<"+++++++++++++++++++++++++++"<<__FILE__<<':'<<__LINE__<<'['<<__FUNCTION__<<']'<<"EXPECT_TRUE fail."<<std::endl;\
        ++Status::c();\
    }\
}while(false)

#define EXPECT_FALSE(a) \
do {\
    if ((a)) {\
        std::cout<<std::endl<<"+++++++++++++++++++++++++++"<<__FILE__<<':'<<__LINE__<<'['<<__FUNCTION__<<']'<<"EXPECT_FALSE fail."<<std::endl;\
        ++Status::c();\
    }\
}while(false)

#endif

