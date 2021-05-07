#pragma once

#if defined(_MSC_VER)
#   define %macro_namespace%_API_IMPORT __declspec(dllimport)
#   define %macro_namespace%_API_EXPORT __declspec(dllexport)
#   define %macro_namespace%_SUPPRESS_EXPORT_WARNING __pragma(warning(push)) __pragma(warning(disable: 4251 4275))
#   define %macro_namespace%_RESTORE_EXPORT_WARNING __pragma(warning(pop))
#elif defined(__GNUC__)
#   define %macro_namespace%_API_IMPORT
#   define %macro_namespace%_API_EXPORT __attribute__((visibility("default")))
#   define %macro_namespace%_SUPPRESS_EXPORT_WARNING
#   define %macro_namespace%_RESTORE_EXPORT_WARNING
#else
#   define %macro_namespace%_API_IMPORT
#   define %macro_namespace%_API_EXPORT
#   define %macro_namespace%_SUPPRESS_EXPORT_WARNING
#   define %macro_namespace%_RESTORE_EXPORT_WARNING
#endif

#if defined(%macro_namespace%_BUILD_SHARED)
#   ifdef %macro_namespace%_EXPORT_SHARED
#       define %macro_namespace%_API %macro_namespace%_API_EXPORT
#   else
#       define %macro_namespace%_API %macro_namespace%_API_IMPORT
#   endif
#else
#   define %macro_namespace%_API
#endif
