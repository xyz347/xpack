xpack
====
[English](README-en.md)
* 用于在C++结构体和json/xml/yaml/bson/mysql/sqlite之间互相转换
* 只有头文件, 无需编译库文件，所以也没有Makefile。
* 支持bson，依赖于`libbson-1.0`，需自行安装。**未经充分测试**，具体请参考[README](README-bson.md)
* 支持MySQL，依赖于`libmysqlclient-dev`，需自行安装。**未经充分测试**
* 支持Sqlite，依赖于[libsqlite3](https://cppget.org/libsqlite3)，需自行安装。**未经充分测试**
* 支持yaml，依赖于[yaml-cpp](https://github.com/jbeder/yaml-cpp)，需自行安装。**未经充分测试**
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
* [union](#union)
* [不定类型](#不定类型)
* [数组](#数组)
* [第三方类和结构体](#第三方类和结构体)
* [格式化缩进](#格式化缩进)
* [XML数组](#xml数组)
* [CDATA](#cdata)
* [Qt支持](#qt支持)
* [MySQL](#mysql)
* [重要说明](#重要说明)

基本用法
----
- 结构体后面用XPACK宏包含各个变量，XPACK内还需要一个字母，不同字母的意义请参考[FLAG](#flag)
- 用xpack::json::encode把结构体转json
- 用xpack::json::decode把json转结构体
```C++
#include <iostream>
#include "xpack/json.h" // Json包含这个头文件，xml则包含xpack/xml.h

using namespace std;

struct User {
    int id;
    string  name;
    XPACK(O(id, name)); // 添加宏定义XPACK在结构体定义结尾
};

int main(int argc, char *argv[]) {
    User u;
    string data = "{\"id\":12345, \"name\":\"xpack\"}";

    xpack::json::decode(data, u);          // json转结构体
    cout<<u.id<<';'<<u.name<<endl;

    string json = xpack::json::encode(u);  // 结构体转json
    cout<<json<<endl;

    return 0;
}
```

容器支持
----
目前支持下列容器(std)
- vector
- set
- list
- map<string, T>
- map<integer, T> // 仅JSON，XML不支持
- unordered_map<string, T> (需要C++11支持)
- shared_ptr (需要C++11支持)

FLAG
----
宏XPACK里面，需要用字母将变量包含起来，比如XPACK(O(a,b))，XPACK可以包含多个字母，每个字母可以包含多个变量。目前支持的字母有：
- X。格式是X(F(flag1, flag2...), member1, member2,...) F里面包含各种FLAG，目前支持的有：
    - 0 没有任何FLAG
    - OE omitempty，encode的时候，如果变量是0或者空字符串或者false，则不生成对应的key信息
    - EN empty as null, 用于json的encode，OE是直接不生成empty的字段，EN则是生成一个null
    - M mandatory，decode的时候，如果这个字段不存在，则抛出异常，用于一些id字段。
    - ATTR attribute，xml encode的时候，把值放到attribute里面。
    - SL single line, json encode的时候，对于数组，放在一行里面
- C。格式是C(customcodec, F(flag1,flags...), member1, member2,...)用于自定义编解码函数，详情请参考[自定义编解码](#自定义编解码)
- O。等价于X(F(0), ...) 没有任何FLAG。
- M。等价于X(F(M)，...) 表示这些字段是必须存在的。
- A。[别名](#别名)，A(member1, alias1, member2, alias2...)，用于变量和key名不一样的情况
- AF。带FLAG的[别名](#别名)，AF(F(flag1, flag2,...), member1, alias1, member2, alias2...)
- B。[位域](#位域)，B(F(flag1, flag2, ...), member1, member2, ...) **位域不支持别名**
- I。[继承](#继承)，I(baseclass1, baseclass2....)，里面放父类
- E。[枚举](#枚举):
    - 如果编译器支持C++11，不需要用E，枚举可以放X/O/M/A里面。
    - 否则枚举只能放E里面，不支持别名

别名
----
- 用于变量名和key名不一致的场景
- 格式是A(变量，别名....)或者AF(F(flags), 变量，别名....)，别名的格式是"x t:n"的格式
    - x表示全局别名，t表示类型(目前支持json、xml、bson)，n表示类型下的别名
    - 全局别名可以没有，比如`json:_id`是合法的
    - 类型别名可以没有，比如`_id`是合法的
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
- 使用"I"来包含父类。需要用到父类的变量就包含，用不到可以不包含。
- 父类的父类也需要包含，比如class Base; class Base1:public Base; class Base2:public Base1;那么在Base2中需要I(Base1, Base)
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
应用场景
- 有些基础类型想用自定义方式编码，比如用字符串的方式来编码整数/浮点数
- 部分类型可能不想按结构体变量逐个编码，比如定义了一个时间结构体：
```C++
struct Time {
    long ts; //unix timestamp
};
```
并不希望编码成{"ts":1218196800} 这种格式，而是希望编码成"2008-08-08 20:00:00"这种格式。

这里有两种方式：
- 使用xtype，可以参考[例子](example/xtype.cpp)
- 用C来包含需要自定义编解码的变量（下面简称C方法），可以参考[例子](example/custom.cpp)

两种方法本质上都是自己去实现encode/decode，但是有以下区别：
- xtype是类型级别的，也就是一旦某个类型用xtype封装之后，那么自定义的encode/decode就对这个类型生效。xtype无法作用于基本类型（int/string等）
- C方法能支持基本类型(int/string等）和非基本类型，但是仅作用于用C包含的变量，比如int a;int b; O(a), C(custome_int, F(0), b);那么a还是用默认的编解码，b才是用自定义的编解码。

1. xtype优先于XPACK宏，也就是定义了xtype的，会优先使用xtype的encode/decode
2. C方法优先于xtype，也就是用C包含的变量，一定会用C里面指定的编解码方法。

用这两个特性，可以实现一些比较灵活的编解码控制，比如这个[例子](example/xtype_advance.cpp)实现了一个根据变量情况来编码的功能，如果Sub.type==1则encode seq1，否则encode seq2. `__x_pack_decode`和`__x_pack_encode`是XPACK宏给结构体添加的decode/encode函数，自定义编解码函数可以通过这些函数调用xpack默认的编解码功能。

union
----
可以使用[自定义编解码](#自定义编解码)来处理联合体，可以参考[范例](example/union1.cpp)

数组
----
- decode的时候如果元素个数超过数组的长度，会截断
- char数组按有\0结束符处理

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

不定类型
----
- 用于json的schema不确定的场景
- 用[xpack::JsonData](json_data.h)来接收这些信息
- 可以参考[例子](example/json-data.cpp)
- xpack::JsonData主要的方法有：
    - Type。用于获取类型
    - IsXXX系列函数。用于判断是否是某种类型，基本等价于return Type()==xxxx;
    - GetXXX系列函数。用来提取值。
    - 重载bool。用来判断是否是一个合法的JsonData。
    - Size。用于数组类型判断元素的个数
    - `operator [](size_t index)` 用来取数组的第index个元素（从0开始）
    - `operator [](const char *key)` 用来根据key取Object类型的元素
    - Begin。用来遍历Object的元素，取第一个。
    - Next。配合Begin使用，获取下一个元素。
    - Key。配置Begin和Next使用，遍历的时候获取Key

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
- 数组默认会用变量名作为元素的标签，比如"ids":[1,2,3]，对应的xml是:
``` xml
<ids>
    <ids>1</ids>
    <ids>2</ids>
    <ids>3</ids>
</ids>
```
- 可以用别名的方式来控制数组的元素的标签，比如A(ids,"xml:ids,vl@id")，vl后面跟着一个@xx，xx就是数组的标签，生成的结果就是：
``` xml
<ids>
    <id>1</id>
    <id>2</id>
    <id>3</id>
</ids>
```
- 如果希望数组直接展开，而不是在外面包一层，可以用别名加"sbs"的flag来实现，比如A(ids, "xml:ids,sbs")，**注意，sbs标签仅能用于数组，其他地方使用可能会崩溃**
``` xml
<ids>1</ids>
<ids>2</ids>
<ids>3</ids>
```

CDATA
----
- 对于CDATA类型，需要用"cdata"的flag来实现，比如A(data, "xml:data,cdata")
- cdata只能用std::string来接收
- 如果变量对应的xml不是CDATA结构，会按普通字符串来处理比如`<data>hello</data>`也可以解析成功

Qt支持
----
- 修改config.h，开启XPACK_SUPPORT_QT这个宏(或者在编译选项开启)
- 当前支持 QString/QMap/QList/QVector 

MySQL
----
- 目前仅支持decode，还不支持encode
- 未经过完整测试，使用需要谨慎
- 当前支持类型有：
    - string。简单测试。
    - 整型。简单测试。
    - 浮点型。未测试。
    - 用整型（比如time_t)接收TIME/DATETIME/TIMESTAMP。未测试。
    - 自定义类型转换，is_xpack_mysql_xtype，类似于xtype。未测试。
- api有两个(xpack::mysql::) ：
    - `static void decode(MYSQL_RES *result, T &val)`
        - 用来将MYSQL_RES转成结构体或者vector<>，如果是非vector，则只转换第一个row
    - `static void decode(MYSQL_RES *result, const std::string&field, T &val)`
        - 用来解析某个字段，用于只想获得某个字段内容的场景，比如select id from mytable where name = lilei，只想获得id信息。val支持vector


重要说明
----
- 变量名尽量不要用__x_pack开头，不然可能会和库有冲突。
- vc6不支持。
- msvc没有做很多测试，只用2019做过简单测试。
- json的序列化反序列化用的是[rapidjson](https://github.com/Tencent/rapidjson)
- xml的反序列化用的是[rapidxml](http://rapidxml.sourceforge.net)
- xml的序列化是我自己写的，没有参考RFC，可能有和标准不一样的地方.
- 有疑问可以加QQ群878041110
