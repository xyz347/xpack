#ifndef __X_PACK_RAPIDJSON_CUSTOME_H
#define __X_PACK_RAPIDJSON_CUSTOME_H

#ifndef RAPIDJSON_NOEXCEPT_ASSERT
  #include <cassert>
  #define RAPIDJSON_NOEXCEPT_ASSERT(x) assert(x)
#endif

#ifndef RAPIDJSON_ASSERT
  #include <stdexcept>
  #define RAPIDJSON_ASSERT(x) if(!(x)) throw std::runtime_error(#x)
#endif

#ifndef RAPIDJSON_HAS_STDSTRING
  #define RAPIDJSON_HAS_STDSTRING 1
#endif


#define RAPIDJSON_WRITE_DEFAULT_FLAGS kWriteNanAndInfFlag

#endif

