
#ifndef XLLR_JVM_EXPORTS_H
#define XLLR_JVM_EXPORTS_H

#ifdef SHARED_EXPORTS_BUILT_AS_STATIC
#  define XLLR_JVM_EXPORTS
#  define XLLR_JVM_NO_EXPORT
#else
#  ifndef XLLR_JVM_EXPORTS
#    ifdef xllr_jvm_EXPORTS
        /* We are building this library */
#      define XLLR_JVM_EXPORTS __declspec(dllexport)
#    else
        /* We are using this library */
#      define XLLR_JVM_EXPORTS __declspec(dllimport)
#    endif
#  endif

#  ifndef XLLR_JVM_NO_EXPORT
#    define XLLR_JVM_NO_EXPORT 
#  endif
#endif

#ifndef XLLR_JVM_DEPRECATED
#  define XLLR_JVM_DEPRECATED __declspec(deprecated)
#endif

#ifndef XLLR_JVM_DEPRECATED_EXPORT
#  define XLLR_JVM_DEPRECATED_EXPORT XLLR_JVM_EXPORTS XLLR_JVM_DEPRECATED
#endif

#ifndef XLLR_JVM_DEPRECATED_NO_EXPORT
#  define XLLR_JVM_DEPRECATED_NO_EXPORT XLLR_JVM_NO_EXPORT XLLR_JVM_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef XLLR_JVM_NO_DEPRECATED
#    define XLLR_JVM_NO_DEPRECATED
#  endif
#endif

#endif /* XLLR_JVM_EXPORTS_H */

