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

int main(int argc, char *argv[]) {
    vector<int> vi(3);
    vi[0] = 1;
    vi[1] = 2;
    vi[2] = 3;

    xpack::JsonEncoder en;
    xpack::XmlEncoder xn;
    en.ob(NULL).add("vv", vi).add("i",10).add("s","hello").oe();
    xn.ob("root").add("vv", vi).add("i",10).add("s","hello").oe();

    cout<<en.String()<<endl;
    cout<<xn.String()<<endl;
    return 0;
}
