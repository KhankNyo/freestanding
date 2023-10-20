#ifndef FREESTANDING_IEEE754_H
#define FREESTANDING_IEEE754_H



#include "fs_int.h"
#include "fs_endian.h"



#define FS_F32_EXP_POS (23)
#define FS_F32_SIGN_POS (31)
#define FS_F32_EXP (0xFF)
#define FS_F32_MANTISSA ((fs_u32)0x007FFFFF)
#define FS_F32_BIAS (127)

#define FS_F64_EXP_POS (52)
#define FS_F64_SIGN_POS (63)
#define FS_F64_EXP ((fs_u32)0x7FF)
#define FS_F64_EXP_BIAS (0x3FF)



typedef union fs_internal_f32_u32
{
    float f;
    fs_u32 u32;
} fs_fu32;


static const fs_fu32 s_fs_internal_float_endian_test = { 2.0 }; /* 0x40000000 in hex */

/* returns true if float is little endian, false otherwise */
#define FS_FLOAT_ENDIAN_DIFFER() \
    (s_fs_internal_float_endian_test.u32 > 0x00008000)





static int fs_exp_of_f(float f)
{
    fs_fu32 cvt;
    fs_u8 exponent;
    cvt.f = f;
    
    if (FS_FLOAT_ENDIAN_DIFFER())
        cvt.u32 = fs_endian_bswap32(cvt.u32);


    exponent = cvt.u32 >> FS_F32_EXP_POS;
    exponent += FS_F32_BIAS;
    exponent &= FS_F32_EXP;
    return (int)(fs_i8)exponent;
}




static int fs_exp_of_d(double d)
{
    fs_u32 exponent;

#ifdef FS_64BIT_DEFINED
    union {
        double d;
        fs_u64 u64;
    } cvt;
    cvt.d = d;

    if (FS_FLOAT_ENDIAN_DIFFER())
        cvt.u32 = fs_endian_bswap32(cvt.u32);

    exponent = (cvt.u64 >> FS_F64_EXP_POS);
#else
    union {
        double d;
        fs_u32 u32[sizeof(double) / sizeof(fs_u32)];
    } cvt;
    cvt.d = d;


    if (FS_FLOAT_ENDIAN_DIFFER())
        fs_endian_bswap(cvt.u32, sizeof(double));

    if (FS_ENDIAN_IS(FS_ENDIAN_LITTLE))
        exponent = cvt.u32[1];
    else 
        exponent = cvt.u32[0];
    exponent >>= FS_F64_EXP_POS - 32;
#endif /* FS_64BIT_DEFINED */

    exponent += FS_F64_EXP_BIAS;
    exponent &= FS_F64_EXP;
    if (exponent >> 10) /* is signed? then sign extend */
        exponent |= ~(((fs_u32)1 << 10) - 1);
    exponent += FS_F64_EXP_BIAS;

    return (int)exponent;
}





#endif /* FREESTANDING_IEEE_754_H */

