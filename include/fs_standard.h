#ifndef FREESTANDING_STANDARD_H
#define FREESTANDING_STANDARD_H


#if defined(__STDC__)
# define FS_C89
# if defined(__STDC_VERSION__)
#  define FS_C90
#  if (__STDC_VERSION__ >= 199409L)
#   define FS_C94
#  endif
#  if (__STDC_VERSION__ >= 199901L)
#   define FS_C99
#  endif
#  if (__STDC_VERSION__ >= 201112L)
#   define FS_C11
#  endif
#  if (__STDC_VERSION__ >= 201710L)
#   define FS_C17
#  endif
#  if (__STDC_VERSION__ >= 202311L)
#   define FS_C23
#  endif
# endif
#endif



#ifndef uintptr_t
#  ifdef FS_C99
#    define uintptr_t unsigned long long
#  else
#    define uintptr_t unsigned long
#  endif /* C99 */
#endif /* uintptr_t */



#ifndef NULL
#  define NULL ((void*)0)
#endif /* NULL */


#endif /* FREESTANDING_STANDARD_H */

