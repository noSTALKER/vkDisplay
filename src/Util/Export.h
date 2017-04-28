#pragma once

#if defined(UTIL_LIBRARY)
#   define UTIL_API   __declspec(dllexport)
#else
#   define UTIL_API   __declspec(dllimport)
#endif
