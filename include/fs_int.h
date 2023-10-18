#ifndef FREESTANDING_INT_H
#define FREESTANDING_INT_H



/* every C compiler should have this */
#include <limits.h>
#include "fs_standard.h"


#ifndef FS_USIZE_DEFINED
#define FS_USIZE_DEFINED
#  if FS_C99
typedef unsigned long long fs_size;
#  else /* C89 */
typedef unsigned long fs_size;
#  endif /* FS_C99 */
#endif /* FS_USIZE_DEFINED */



#ifndef FS_8BIT_DEFINED
#define FS_8BIT_DEFINED
#  if CHAR_BIT != 8
#    error "fuck you"
#  endif
typedef unsigned char fs_u8;
typedef char fs_i8;
#endif /* FS_8BIT_DEFINED */


#ifndef FS_16BIT_DEFINED
#define FS_16BIT_DEFINED
#  if USHRT_MAX == 0xffffu
typedef unsigned short fs_u16;
typedef short fs_i16;
#  elif UINT_MAX == 0xffffu
typedef unsigned int fs_u16;
typedef int fs_i16;
#  else
#    error "cannot defined 16 bit integer type in '" __FILE__ "'"
#  endif
#endif /* FS_16BIT_DEFINED */



#ifndef FS_32BIT_DEFINED
#define FS_32BIT_DEFINED
#  if ULONG_MAX == 0xfffffffflu
typedef unsigned long fs_u32;
typedef long fs_i32;
#  elif UING_MAX == 0xfffffffflu
typedef unsigned int fs_u32;
typedef int fs_i32;
#  else
#    error "cannot define 32 bit integer type in '" __FILE__ "'"
#  endif
#endif /* FS_U32_DEFINED */


#ifdef FS_C99
#  ifndef FS_64BIT_DEFINED
#  define FS_64BIT_DEFINED
#    if ULLONG_MAX == 0xffffffffffffffffllu
typedef unsigned long long fs_u64;
typedef long long fs_i64;
#    elif ULONG_MAX == 0xffffffffffffffffllu
typedef unsigned long fs_u64;
typedef long fs_i64;
#    else
#      error "cannot define 64 bit integer type in '" __FILE__ "'"
#    endif
#  endif /* FS_64BIT_DEFINED */
#endif /* FS_C99 */




#endif /* FREESTANDING_INT_H */

