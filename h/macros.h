#pragma once

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

#if defined(__GNUC__)
#define UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define UNREACHABLE() __assume(0)
#else
#define UNREACHABLE()
#endif
