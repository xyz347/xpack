xpack 
==== 
* Used to convert between C++ structure and json/xml, bson is supported in [xbson](https://github.com/xyz347/xbson). 
* Only header files, no need to compile library files, so there is no Makefile. 
* Support MySQL, depends on `libmysqlclient-dev`, need to install by yourself. **not fully tested**
* Support Sqlite, depends on [libsqlite3](https://cppget.org/libsqlite3), need to install it yourself. **not fully tested**
* Support yaml, depend on [yaml-cpp](https://github.com/jbeder/yaml-cpp), need to install it yourself. **not fully tested**
* For details, please refer to the example

------ 
* [Quick start](#quick-start)
* [Container Support](#container-support)
* [FLAG](#flag)
* [Alias](#alias)
* [Bitfield](#bitfield)
* [Inherit](#inherit)
* [enum](#enum)
* [Custom codec](#custom-codec)
* [union](#union)
* [Schema indeterminate type](#schema-indeterminate-type)
* [Define macro outside the structure](#define-macro-outside-the-structure)
* [Array](#array)
* [Format indentation](#format-indentation)
* [XML array](#xml-array)
* [CDATA](#cdata)
* [Qt support](#qt-support)
* [Important note](#important-note)

Quick start
----
- Use macro `XPACK`(or `XPACK_OUT` if you don't want to change the structure) include structure members
- Use xpack::json::encode to convert structure to json
- Use xpack::json::decode to convert json to structure

```C++
#include <iostream>
#include "xpack/json.h" // include this header file for json or "xpack/xml.h" for xml

using namespace std;

struct User {
    int id;
    string  name;
    XPACK(O(id, name)); // Add XPACK at the end of structure, up to 99 'O' can be added, and each 'O' can contain up to 99 members
};

int main(int argc, char *argv[]) {
    User u;
    string data = "{\"id\":12345, \"name\":\"xpack\"}";

    xpack::json::decode(data, u);          // json to structure
    cout<<u.id<<';'<<u.name<<endl;

    string json = xpack::json::encode(u);  // structure to json
    cout<<json<<endl;

    return 0;
}
```

Container Support
----
The following containers are currently supported (std)
- vector
- set
- list
- map<string, T>
- map<integer, T> // only for json
- unordered_map<string, T> (need C++11)
- shared_ptr (need C++11)

FLAG
----
In the macro `XPACK`, variables need to be enclosed with letters, such as `XPACK(O(a,b))`, `XPACK` can contain multiple letters, and each letter can contain multiple variables. Currently supported letters are:
- X Usage: X(F(flag1, flag2...), member1, member2,...) Multiple flags can be written in 'F', currently supported flags are:
    - 0 no flag
    - OE omitempty. When encoding, if the variable is 0 or an empty string or false, the variable is ignored
    - EN empty as null. When json encoding, encode empty variable as null(OE is ignore the variable)
    - M mandatory. When decoding, if the key corresponding to the variable does not exist, an exception is thrown
    - ATTR attribute. When xml encode, put the value in attribute
    - SL single line, When json encoding, put vector in single line
- C Usage: C(customcodec, F(flag1,flags...), member1, member2,...) For custom codec function, please refer to [Custom codec](#custom-codec) for details
- O Usage: O(member1, member2, ...) Same as X(F(0), member1, member2, ...) no flags
- M Usage: M(member1, member2, ...) Same as X(F(M), member1, member2, ...) mandatory
- A Usage: A(member1, alias1, member2, alias2...) Used when the key and the variable name are different, please refer to [Alias](#alias) for details
- AF Usage: AF(F(flag1, flag2,...), member1, alias1, member2, alias2...) Alias with flags
- B Usage: B(F(flag1, flag2, ...), member1, member2, ...) For bitfield, please refer to [Bitfield](#bitfield) for details. **bitfield not support alias**
- I Usage: I(baseclass1, baseclass2....), baseclassx is parent class, please refer to [Inherit](#inherit) for details
- E Usage: E(member1, member2) For enumeration variables that do not support C++11. **E not support alias**

Alias
----
- Used when the key and the variable name are different
- Usage: A(member1, alias1, member2, alias2...) or AF(F(flag1, flag2,...), member1, alias1, member2, alias2...), alias's format is "x t:n"
    - x stands for global alias, not required
    - t stands for type(currently support json/xml/bson), n stands for type alias, type alias is not required
    - The order of precedence is: type alias, global alias, variable name
    - For example, `id bson:_id`, in bson will use `_id` and json/xml will use `id`
- Type alias can take some type flags, format is "t:n,flag1,flag2,...", currently supported flags are:
    - cdata. For xml, indicates that it is cdata
    - sbs. For xml vector, In normal, `vector<int> nums` will be encoded into `<nums><nums>1</nums><nums>2</nums><nums>`, in sbs is `<nums>1</nums><nums>2</nums>`

``` C++
#include <iostream>
#include "xpack/json.h"

using namespace std;

struct Test {
    long uid;
    string  name;
    XPACK(A(uid, "id"), O(name)); // "uid" use alias "id"
};

int main(int argc, char *argv[]) {
    Test t;
    string json="{\"id\":123, \"name\":\"Pony\"}";

    xpack::json::decode(json, t); 
    cout<<t.uid<<endl;
    return 0;
}

```

Bitfield
----
- Use 'B' include bitfield members. **bitfield not support alias**

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

Inherit
----
- Use 'I' include parent classes. If you don't need variables from the parent class, you don't need use 'I'
- Parent class need add macro XPACK/XPACK_OUT

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

enum
----
- If support C++11, enum is same as other type, use X/O/M/A is ok
- Otherwise need to use 'E', usage: E(F(...), member1, member2, ...)

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

Custom codec
----
Used in scenarios where you want to define your own codec method, such as:
- Wish to encode numeric types in string or hexadecimal
- Don't want to encode the struct variable by variable:
```C++
struct Time {
    long ts; //unix timestamp
};
```
Don't want to encode to {"ts":1218196800} but "2008-08-08 20:00:00"

There are two ways to achieve:
- Use xtype. You can refer to this [example](example/xtype.cpp)
- Use 'C' include members need to custome codec(Hereinafter referred to as the `C method`), You can refer to this [example](example/custom.cpp)

Both methods need to implement custom codec, but there are some differences:
- xtype is at the type level, that is, once a certain type is encapsulated with xtype, the custom encode/decode will take effect for this type. xtype cannot be used for basic types (int/string, etc.)
- The C method can support basic types (int/string, etc.) and non-basic types, but only works on variables contained in C, such as 
```C++
int a;
int b; 
XPACK(O(a), C(custome_int, F(0), b ))
```
Then "a" still uses the default codec, and "b" uses the custom codec.

1. xtype takes precedence over the XPACK macro, that is, if xtype is defined, the encode/decode of xtype will be used first
2. The C method takes precedence over xtype, that is, the variables contained in C must use the codec method specified in C

With these two features, some more flexible codec control can be realized. For example, this [example](example/xtype_advance.cpp) realizes a function of encoding according to the variable situation. If Sub.type==1, then encode seq1, Otherwise encode seq2. `__x_pack_decode` and `__x_pack_encode` are the decode/encode functions added to the structure by the XPACK macro, and the custom codec function can call the default codec function of xpack through these functions.

union
----
Unions can be handled using [Custom codec](#custom-codec). You can refer to this [example](example/union1.cpp)

Array
----
- When decoding, if the number of elements exceeds the length of the array, it will be truncated
- The char array needs to have a '\0' terminator

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

Schema indeterminate type
----
- Json only
- Use [xpack::JsonData](json_data.h) for this kind of content
- You can refer to this [example](example/json-data.cpp)
- The main APIs for xpack::JsonData are:
    - Type. To get the field type
    - IsXXX. To check is the field type is xxx, same as `return Type()==xxxx;`
    - GetXXX. To get the field value
    - operator bool. To check is't a valid xpack::JsonData
    - Size. To get the array's elements number 
    - `operator [](size_t index)`. Used to get the index element in array
    - `operator [](const char *key)`. Used to get the element by key in object
    - Iterator:
        - Begin. Get the first element's iterator
        - Next. Get the next iterator
        - Key. Get the key of iterator
        - Val. Get the value of iterator

Define macro outside the structure
----
- Use `XPACK_OUT` instead of `XPACK` to include structure members
- `XPACK_OUT` must define in global namespace

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

Format indentation
----
- The default encode is not indented, and the readability is not good
- If indentation is required, it can be controlled by the last two parameters of encode:
	- indentCount Indicates the number of characters for indentation, <0 means no indentation, 0 means newline but no indentation
	- indentChar Characters that represent indentation, use spaces or tabs

XML array
----
- Arrays use variable names as element labels by default, such as "ids":[1,2,3] will be encoded as:
``` xml
<ids>
    <ids>1</ids>
    <ids>2</ids>
    <ids>3</ids>
</ids>
```
- You can use alias to change the label name. For example: A(ids,"xml:ids,vl@id"), vl@id means to use id as label:
``` xml
<ids>
    <id>1</id>
    <id>2</id>
    <id>3</id>
</ids>
```
- If want to encode array element side by side, can use `sbs` flag. For example: `A(ids, "xml:ids,sbs")`. **Note that the sbs tag can only be used for arrays(vector), and it may crash if used elsewhere**
``` xml
<ids>1</ids>
<ids>2</ids>
<ids>3</ids>
```

CDATA
----
- For xml CDATA, need to use `cdata` flag. For example: A(data, "xml:data,cdata")
- variable type must be std::string
- Non-cdata types can also add cdata flag, which will be processed as ordinary strings

Qt support
----
- Modify [config.h](config.h) to enable XPACK_SUPPORT_QT(or enable it in compile flags)
- Currently supports: QString/QMap/QList/QVector 


Important note
----
- Try not to start the variable name with __x_pack, otherwise it may conflict with the library.
- vc6 is not supported
- msvc has only been tested on vs2019
- The serialization and deserialization of json uses [rapidjson](https://github.com/Tencent/rapidjson) (November 2018 edition)
- The deserialization of xml uses [rapidxml](http://rapidxml.sourceforge.net)
- The serialization of xml is written by myself, without reference to RFC, there may be some differences from the standard.
