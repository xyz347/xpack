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

#include <iostream>
#include "xpack/json.h"
#include "xpack/xml.h"

using namespace std;

struct User {
    int64_t id;
    string  name;
    string  mail;
    std::shared_ptr<string> bio;
    User(int64_t i=0, const string& n="", const string& m=""):id(i),name(n),mail(m){}
    XPACK(O(id, name, mail, bio)); // 添加宏定义XPACK在结构体定义结尾
};


int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    User u1(1, "Jack", "jack@xpack.com");
    User u2;
    User u3;

    string json = xpack::json::encode(u1);      // 结构体转json
    string xml = xpack::xml::encode(u1, "root");
    cout<<"========================"<<endl;
    cout<<json<<endl;
    cout<<xml<<endl;

    cout<<"========================"<<endl;
    xpack::json::decode(json, u2);
    cout<<xpack::json::encode(u2)<<endl;
    xpack::xml::decode(xml, u3);
    cout<<xpack::xml::encode(u3, "root")<<endl;

    cout<<"========================"<<endl;
    u1.bio.reset(new std::string("farmer"));
    json = xpack::json::encode(u1);      // 结构体转json
    xml = xpack::xml::encode(u1, "root");
    cout<<json<<endl;
    cout<<xml<<endl;

    return 0;
}
