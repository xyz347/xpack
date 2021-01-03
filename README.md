xpack
====
* 用于在C++结构体和json/xml之间互相转换, bson在[xbson](https://github.com/xyz347/xbson)中支持。
* 只需要头文件, 无需编译库文件。
* 具体可以参考example的例子

------
* [基本用法](#基本用法)
* [容器支持](#容器支持)
* [FLAG](#flag)
* [别名](#别名)
* [位域](#位域)
* [继承](#继承)
* [枚举](#枚举)
* [自定义编解码](#自定义编解码)
* [char数组](#char数组)
* [第三方类和结构体](#第三方类和结构体)
* [格式化缩进](#格式化缩进)
* [XML数组](#xml数组)
* [Qt支持](#qt支持)
* [重要说明](#重要说明)

基本用法
----
```C++
#include <iostream>
#include "xpack/json.h" // 包含这个头文件

using namespace std;

struct User {
    int64_t id;
    string  name;
    string  mail;
    User(int64_t i=0, const string& n="", const string& m=""):id(i),name(n),mail(m){}
    XPACK(O(id, name, mail)); // 添加宏定义XPACK在结构体定义结尾
};

struct Group {
    string  name;
    int64_t master;
    vector<User> members;
    XPACK(O(name, master, members)); // 添加宏定义XPACK在结构体定义结尾
};

int main(int argc, char *argv[]) {
    Group g;
    g.name = "C++";
    g.master = 2019;
    g.members.resize(2);
    g.members[0] = User(1, "Jack", "jack@xpack.com");
    g.members[1] = User(2, "Pony", "pony@xpack.com");

    string json = xpack::json::encode(g);  	// 结构体转json
    cout<<json<<endl;

    Group n;
    xpack::json::decode(json, n); 			// json转结构体
    cout<<n.name<<endl;

    vector<int> vi;
    xpack::json::decode("[1,2,3]", vi); 	// 直接转换vector
    cout<<vi.size()<<','<<vi[1]<<endl;

    map<string, int> m;
    xpack::json::decode("{\"1\":10, \"2\":20}", m); // 直接转换map
    cout<<m.size()<<','<<m["2"]<<endl;

    return 0;
}
```
步骤有：
- 引用头文件 "xpack/json.h"
- 添加宏定义XPACK在结构体结尾, 里面用"O"包含所有变量
- 用xpack::json::encode将结构体转json
- 用xpack::json::decode将json转结构体

容器支持
----
目前支持下列容器(std)
- vector
- set
- list
- map<string, T>
- unordered_map<string, T> (需要C++11支持)
- shared_ptr (需要C++11支持)

FLAG
----
宏XPACK里面，需要用字母将变量包含起来，比如XPACK(O(a,b))，目前支持的字母有：
- X。格式是X(F(flag1, flag2...), member1, member2,...) F里面包含各种FLAG，目前支持的有：
	- 0 没有任何FLAG
	- OE omitempty，encode的时候，如果变量是0或者空字符串或者false，则不生成对应的key信息
	- M mandatory，decode的时候，如果这个字段不存在，则抛出异常，用于一些id字段。
    - ATTR attribute，xml encode的时候，把值放到attribute里面。
- O。等价于X(F(0), ...) 没有任何FLAG。
- M。等价于X(F(M)，...) 表示这些字段是必须存在的。
- A。[别名](#别名)，A(member1, alias1, member2, alias2...)
- AF。带FLAG的[别名](#别名)，AF(F(flag1, flag2,...), member1, alias1, member2, alias2...)
- B。[位域](#位域)，B(F(flag1, flag2, ...), member1, member2, ...) **位域不支持别名**
- I。[继承](#继承)，I(baseclass1, baseclass2....)
- E。[枚举](#枚举):
	- 如果编译器支持C++11，不需要用E，枚举可以放X/O/M/A里面。
	- 否则枚举只能放E里面，不支持别名

别名
----
- 用于变量名和key名不一致的场景
- 格式是A(变量，别名....)或者AF(F(flags), 变量，别名....)，别名的格式是"x t:n"的格式
	- x表示全局别名，t表示类型(目前支持json)，n表示类型下的别名
	- 全局别名可以没有，比如"json:_id"是合法的
	- 类型别名可以没有，比如"_id"是合法的
	- 有类型别名优先用类型别名，否则用全局别名，都没有，则用变量名

``` C++
#include <iostream>
#include "xpack/json.h"

using namespace std;

struct Test {
    long uid;
    string  name;
    XPACK(A(uid, "id"), O(name)); // "uid"的别名是"id"
};

int main(int argc, char *argv[]) {
    Test t;
    string json="{\"id\":123, \"name\":\"Pony\"}";

    xpack::json::decode(json, t); 
    cout<<t.uid<<endl;
    return 0;
}

```

位域
----
- 使用"B"来包含位域变量，**位域不支持别名**

``` C++
#include <iostream>
#include "xpack/json.h"

using namespace std;

struct Test {
    short ver:8;
    short len:8;
    string  name;
    XPACK(B(F(0), ver, len), O(name)); 
};

int main(int argc, char *argv[]) {
    Test t;
    string json="{\"ver\":4, \"len\":20, \"name\":\"IPv4\"}";

    xpack::json::decode(json, t);
    cout<<t.ver<<endl;
    cout<<t.len<<endl;
    return 0;
}
```

继承
----
- 使用"I"来包含父类
- 所有父类都需要包含，比如class Base; class Base1:public Base; class Base2:public Base1;那么在Base2中需要I(Base1, Base)
- 父类也需要定义XPACK/XPACK_OUT宏。

```C++
#include <iostream>
#include "xpack/json.h"

using namespace std;

struct P1 {
    string mail;
    XPACK(O(mail));
};

struct P2 {
    long version;
    XPACK(O(version));
};

struct Test:public P1, public P2 {
    long uid;
    string  name;
    XPACK(I(P1, P2), O(uid, name)); 
};

int main(int argc, char *argv[]) {
    Test t;
    string json="{\"mail\":\"pony@xpack.com\", \"version\":2019, \"id\":123, \"name\":\"Pony\"}";

    xpack::json::decode(json, t);
    cout<<t.mail<<endl;
    cout<<t.version<<endl;
    return 0;
}

```

枚举
----
- 如果编译器支持C++11，则枚举和普通变量名一样，放X/O/M/A里面皆可。
- 否则需要放到E里面，格式是E(F(...), member1, member2, ...)

```C++
#include <iostream>
#include "xpack/json.h"

using namespace std;

enum Enum {
    X = 0,
    Y = 1,
    Z = 2,
};

struct Test {
    string  name;
    Enum    e;
    XPACK(O(name), E(F(0), e)); 
};

int main(int argc, char *argv[]) {
    Test t;
    string json="{\"name\":\"IPv4\", \"e\":1}";

    xpack::json::decode(json, t);
    cout<<t.name<<endl;
    cout<<t.e<<endl;
    return 0;
}

```

自定义编解码
----
应用场景：部分类型可能不想按结构体变量逐个编码，比如定义了一个时间结构体：
```C++
struct Time {
    long ts; //unix timestamp
};
```
并不希望编码成{"ts":1218196800} 这种格式，而是希望编码成"2008-08-08 20:00:00"这种格式，这个时候就可以用自定义编解码实现。

- 可以参考[例子](example/xtype.cpp)

char数组
----
- **缺省是不支持char数组的**
- 修改[config.h](config.h)，开启XPACK_SUPPORT_CHAR_ARRAY这个宏即可。也可以直接在编译选项加上这个定义。
- **除了char，其他类型不支持数组**

```C++
#include <iostream>
#include "xpack/json.h"

using namespace std;


struct Test {
    char  name[64];
    char  email[64];
    XPACK(O(name, email)); 
};

int main(int argc, char *argv[]) {
    Test t;
    string json="{\"name\":\"Pony\", \"email\":\"pony@xpack.com\"}";

    xpack::json::decode(json, t);
    cout<<t.name<<endl;
    cout<<t.email<<endl;
    return 0;
}
```

第三方类和结构体
----
- 用XPACK_OUT而非XPACK来包含变量
- XPACK_OUT必须定义在全局命名空间

```c++
#include <sys/time.h>
#include <iostream>
#include "xpack/json.h"

using namespace std;

/*
struct timeval {
    time_t      tv_sec;
    suseconds_t tv_usec;
};
*/

// timeval is thirdparty struct
XPACK_OUT(timeval, O(tv_sec, tv_usec));

struct T {
    int  a;
    string b;
    timeval t;
    XPACK(O(a, b, t));
};


int main(int argc, char *argv[]) {
    T t;
    T r;
    t.a = 123;
    t.b = "xpack";
    t.t.tv_sec = 888;
    t.t.tv_usec = 999;
    string s = xpack::json::encode(t);
    cout<<s<<endl;
    xpack::json::decode(s, r);
    cout<<r.a<<','<<r.b<<','<<r.t.tv_sec<<','<<r.t.tv_usec<<endl;
    return 0;
}
```

格式化缩进
----
- encode缺省生成的json/xml是没有缩进的，适合程序使用，如果让人读，可以进行缩进。
- encode的最后两个参数控制
	- indentCount 表示缩进的字符数，<0表示不缩进，0则是换行但是不缩进
	- indentChar 表示缩进的字符，用空格或者制表符

XML数组
----
- 数组会用"x"作为标签，比如"ids":[1,2,3]，对应的xml是:
``` xml
<ids>
    <x>1</x>
    <x>2</x>
    <x>3</x>
</ids>
```

Qt支持
----
- 修改config.h，开启XPACK_SUPPORT_QT这个宏(或者在编译选项开启)
- 当前支持 QString/QMap/QList/QVector 


重要说明
----
- 变量名尽量不要用__x_pack开头，不然可能会和库有冲突。
- vc6不支持。
- msvc没有做很多测试，只用2019做过简单测试。
- json的序列化反序列化用的是[rapidjson](https://github.com/Tencent/rapidjson)
- xml的反序列化用的是[rapidxml](http://rapidxml.sourceforge.net)
- xml的序列化是我自己写的，没有参考RFC，可能有和标准不一样的地方.
- 有疑问可以加QQ群878041110
