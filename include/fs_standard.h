#ifndef FREESTANDING_STANDARD_H
#define FREESTANDING_STANDARD_H


#if defined(__STDC__)
# define PREDEF_STANDARD_C_1989
# if defined(__STDC_VERSION__)
#  define PREDEF_STANDARD_C_1990
#  if (__STDC_VERSION__ >= 199409L)
#   define PREDEF_STANDARD_C_1994
#  endif
#  if (__STDC_VERSION__ >= 199901L)
#   define PREDEF_STANDARD_C_1999
#  endif
#  if (__STDC_VERSION__ >= 201112L)
#   define PREDEF_STANDARD_C_2011
#  endif
#  if (__STDC_VERSION__ >= 201710L)
#   define PREDEF_STANDARD_C_2017
#  endif
#  if (__STDC_VERSION__ >= 202311L)
#   define PREDEF_STANDARD_C_2023
#  endif
# endif
#endif



#ifndef uintptr_t
#  ifdef PREDEF_STANDARD_C_1999
#    define uintptr_t unsigned long long
#  else
#    define uintptr_t unsigned long
#  endif /* C99 */
#endif /* uintptr_t */



#ifndef NULL
#  define NULL ((void*)0)
#endif /* NULL */


#endif /* FREESTANDING_STANDARD_H */

