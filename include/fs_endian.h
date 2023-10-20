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



static fs_u32 fs_endian_bswap32(fs_u32 n)
{
    fs_u32 ret;
    fs_u8 *pn = (fs_u8*)&n;
    fs_u8 *pr = (fs_u8*)&ret;

    pr[0] = pn[3];
    pr[1] = pn[2];
    pr[2] = pn[1];
    pr[3] = pn[0];
    return ret;
}


static void fs_endian_bswap(void *bytes, fs_size count)
{
    fs_size i = 0; 
    fs_u8 *bptr = bytes;
    for (; i < count/2; i += 1)
    {
        fs_u8 tmp = bptr[i];
        bptr[i] = bptr[count - i - 1];
        bptr[count - i - 1] = tmp;
    }
}



static void fs_endian_host_to_little(fs_u8 *buf, fs_size count, unsigned int elem_size)
{
    fs_size i = 0;
    union {
        fs_u32 *dwptr;
        fs_u8 *byteptr;
    } cvt;

    switch (FS_ENDIANNESS())
    {
    default:
    case FS_ENDIAN_LITTLE: break;
    case FS_ENDIAN_BIG:
        for (; i < count * elem_size; i += elem_size)
            fs_endian_bswap(&buf[i], elem_size);
        break;

    case FS_ENDIAN_PDP: /* u32 only */
        if (elem_size == sizeof(fs_u32))
        {
            for (; i < count * sizeof(fs_u32); i += sizeof(fs_u32))
            {
                fs_u32 dword;

                cvt.byteptr = &buf[i];
                dword = *cvt.dwptr;
                *cvt.dwptr = ((dword << 16) & 0xFFFF0000) 
                    | ((dword >> 16) & 0x0000FFFF);
            }
        }
        break;

    case FS_ENDIAN_HONEYWELL: /* u32 only */
        if (elem_size == sizeof(fs_u32))
        {
            for (; i < count * sizeof(fs_u32); i += sizeof(fs_u32))
            {
                fs_u32 dword;

                cvt.byteptr = &buf[i];
                dword = *cvt.dwptr;
                *cvt.dwptr = ((dword << 8) & 0xFF00FF00)
                    | ((dword >> 8) & 0x00FF00FF);
            }
        }
        break;
    }
}


#endif /* FREE_STANDING_ENDIAN_H */

