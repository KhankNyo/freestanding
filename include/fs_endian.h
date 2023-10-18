#ifndef FREESTANDING_ENDIAN_H
#define FREESTANDING_ENDIAN_H


#include "fs_int.h"

static const union {
    fs_u8 bytes[4];
    fs_u32 u32;
} s_fs_internal_endian_test = {{0, 1, 2, 3}};


#define FS_ENDIAN_LITTLE ((fs_u32)0x03020100)
#define FS_ENDIAN_BIG ((fs_u32)0x000102030)
#define FS_ENDIAN_PDP ((fs_u32)0x01000302)
#define FS_ENDIAN_HONEYWELL ((fs_u32)0x02030001)


#define FS_ENDIAN_IS(endian_type) ((s_fs_internal_endian_test).u32 == (endian_type))
#define FS_ENDIANNESS() ((s_fs_internal_endian_test).u32)


#endif /* FREE_STANDING_ENDIAN_H */

