

#ifndef FREESTANDING_TRULY
#  include <errno.h>
#  include <string.h>
#else
#endif /* FREESTANDING_TRULY */

#include "../include/fs_int.h"
#include "../include/fs_snprintf.h"
#include "../include/fs_standard.h"
#include "../include/fs_assert.h"
#include "../include/fs_endian.h"



#ifdef DEBUG_TEST
#  define SNPRINTF_TEST
#endif /* DEBUG_TEST */


#define DEC_BUFSIZE 32
#define HEX_BUFSIZE (sizeof(void*) * 2 + 2)

/* flags */
#define SPACE (1 << 0)
#define PAD_RIGHT (1 << 1)
#define PLUS (1 << 2)
#define ZEROPAD (1 << 3)
#define ALTERNATE_FORM (1 << 4)
#define PRECISION_PROVIDED (1 << 5)
#define CAPITALIZED (1 << 6)


#define VALUE_NEG_POS 7
#define VALUE_ZERO_POS 8
#define VALUE_NEG (1 << VALUE_NEG_POS)
#define VALUE_ZERO (1 << VALUE_ZERO_POS)


#define TO_UPPER_FROM_LOWER(ch) ((ch) - 32)



static const char s_hexchars[17] = "0123456789abcdef";
static const char s_HEXCHARS[17] = "0123456789ABCDEF";





static int is_number(char ch)
{
    return ('0' <= ch) && (ch <= '9');
}

static int is_lower(char ch)
{
    return ('a' <= ch) && (ch <= 'z');
}


static char get_signch(unsigned int flags)
{
    if (flags & VALUE_NEG)
        return '-';
    if (flags & PLUS)
        return '+';
    if (flags & SPACE)
        return ' ';
    return 0;
}


/* if limit is 0, this function will act like strlen */
static unsigned long strlen_up_to(const char *s, unsigned long limit)
{
    unsigned long i = 0;
    if (0 == limit)
    {
        while (s[i] != 0) 
            i += 1;
        return i;
    }
    for (; i < limit; i += 1)
    {
        if (s[i] == 0)
            return i;
    }
    return limit;
}



static void spool_str_rev(char **bufptr, fs_size *left, int *ret, const char *numstr, int len)
{
    int i = len;
    while (i)
    {
        if (*left > 1)
        {
            i -= 1;
            (**bufptr) = numstr[i];
            (*bufptr) += 1;
            (*left) -= 1;
        }
        else i -= 1;
        (*ret) += 1;
    }
}



static void spool_str(char **bufptr, fs_size *left, int *ret, 
    const char *str, int len, unsigned int capitalized)
{
    int i;
    for (i = 0; i < len; i += 1, (*ret) += 1)
    {
        if (*left > 1)
        {
            if (capitalized && is_lower(str[i]))
                (**bufptr) = TO_UPPER_FROM_LOWER(str[i]);
            else
                (**bufptr) = str[i];
            (*bufptr) += 1;
            (*left) -= 1;
        }
    }
}




static void print_pad(char **bufptr, fs_size *left, int *ret, char pad, int count)
{
    int n = count;
    while (n--)
    {
        if (*left > 1)
        {
            (**bufptr) = pad;
            (*bufptr) += 1;
            (*left) -= 1;
        }
        (*ret) += 1;
    }
}


static void print_num_pad(
    char **bufptr, fs_size *left, int *ret, 
    int minw, int precision, unsigned int flags,
    const char *numstr, int len)
{
    int numw = len;
    char signch = get_signch(flags);

    if (0 == precision && (flags & VALUE_ZERO)) numw = 0;
    if (numw < precision) numw = precision;


    if (flags & PAD_RIGHT)
    {
        /* number left, padding right */
        /* [sign][zeros][number][spaces] */

        if (signch) /* print the sign ch */
        {
            numw += 1;
            print_pad(bufptr, left, ret, signch, 1);
        }


        /* number */
        if (precision || !(flags & VALUE_ZERO))
        {
            if (len < precision)
                print_pad(bufptr, left, ret, '0', precision - len);
            spool_str_rev(bufptr, left, ret, numstr, len);
        }


        /* spaces */
        if (numw < minw)
            print_pad(bufptr, left, ret, ' ', minw - numw);
    }
    else
    {
        /* padding left, number right */
        /* [spaces][sign][zeros][number] */

        if ((flags & ZEROPAD) && !(flags & PRECISION_PROVIDED) && (numw < minw))
            numw = minw;
        else if (signch)
            numw += 1;

        /* space pad */
        if (numw < minw)
            print_pad(bufptr, left, ret, ' ', minw - numw);

        /* sign */
        if (signch)
        {
            print_pad(bufptr, left, ret, signch, 1);
            numw -= 1;
        }

        /* zeros */
        if (len < numw)
        {
            print_pad(bufptr, left, ret, '0', numw - len);
        }

        if (precision || !(flags & VALUE_ZERO))
            spool_str_rev(bufptr, left, ret, numstr, len);
    }
}



/* prints the reversed version of val into buf */
static int print_decimal_l(char *buf, int bufsz, unsigned long val)
{
    int len = 0;
    unsigned long value = val;

    if (value == 0 && bufsz > 0)
    {
        buf[0] = '0';
        len = 1;
    }
    else while (len < bufsz && value)
    {
        buf[len] = '0' + (value % 10);
        value /= 10;
        len += 1;
    }

    return len;
}



static int print_hex_l(char *buf, int bufsz, unsigned long value, unsigned int flags)
{
    int len = 0;
    unsigned long val = value;
    const char *lut = s_hexchars;
    char hex = 'x';

    if (flags & CAPITALIZED)
    {
        lut = s_HEXCHARS;
        hex = 'X';
    }

    if (0 == val && bufsz > 0)
    {
        buf[0] = '0';
        len = 1;
    }
    else while (val && len < bufsz)
    {
        buf[len] = lut[val & 0xF];
        val >>= 4;
        len += 1;
    }

    if ((flags & ALTERNATE_FORM) && (len + 2 <= bufsz))
    {
        buf[len] = hex;
        buf[len + 1] = '0';
        len += 2;
    }

    return len;
}







static void print_num_ld(char **bufptr, fs_size *left, int *ret,
    long value, int minw, int precision, unsigned int flags)
{
    char tmp[DEC_BUFSIZE];
    unsigned long abs_val;
    int len;
    unsigned int flags2 = flags;

    flags2 = flags2 | ((value < 0) << VALUE_NEG_POS);
    flags2 = flags2 | ((value == 0) << VALUE_ZERO_POS);

    if (value < 0)
        abs_val = -value;
    else
        abs_val = value;

    len = print_decimal_l(tmp, DEC_BUFSIZE, abs_val);
    print_num_pad(bufptr, left, ret, minw, precision, flags2,  
        tmp, len
    );
}


static void print_num_lu(char **bufptr, fs_size *left, int *ret,
    unsigned long value, int minw, int precision, unsigned int flags)
{
    char tmp[DEC_BUFSIZE];
    int len;
    unsigned int flags2 = flags;
    flags2 = flags2 | ((value == 0) << VALUE_ZERO_POS);

    len = print_decimal_l(tmp, DEC_BUFSIZE, value);
    print_num_pad(bufptr, left, ret, minw, precision, flags2,  
        tmp, len
    );
}


static void print_num_lx(char **bufptr, fs_size *left, int *ret,
    unsigned long value, int minw, int precision, unsigned int flags)
{
    char tmp[DEC_BUFSIZE];
    int len;
    unsigned int flags2 = flags;
    flags2 = flags2 | ((value == 0) << VALUE_ZERO_POS);

    len = print_hex_l(tmp, DEC_BUFSIZE, value, flags2);
    print_num_pad(bufptr, left, ret, minw, precision, flags2, 
        tmp, len
    );
}



static void print_str(char **bufptr, fs_size *left, int *ret, 
    const char *str, int minw, int precision, unsigned int flags)
{
    long width;
    if (flags & PRECISION_PROVIDED)
        width = strlen_up_to(str, (unsigned long)precision);
    else 
        width = strlen_up_to(str, 0);


    if ((width < minw) && !(flags & PAD_RIGHT))
        print_pad(bufptr, left, ret, ' ', minw - width);

    spool_str(bufptr, left, ret, str, width, flags & CAPITALIZED);

    if ((width < minw) && (flags & PAD_RIGHT))
        print_pad(bufptr, left, ret, ' ', minw - width);
}



static void print_chr(char **bufptr, fs_size *left, int *ret,
    char ch, int minw, unsigned int flags)
{
    char character = ch;
    if ((flags & CAPITALIZED) && is_lower(ch))
        character = TO_UPPER_FROM_LOWER(ch);


    if ((1 < minw) && !(flags & PAD_RIGHT))
        print_pad(bufptr, left, ret, ' ', minw - 1);

    print_pad(bufptr, left, ret, character, 1);

    if ((1 < minw) && (flags & PAD_RIGHT))
        print_pad(bufptr, left, ret, ' ', minw - 1);
}




#ifdef FS_C99

/* prints the reversed version of val into buf */
static int print_decimal_ll(char *buf, int bufsz, unsigned long long val)
{
    int len = 0;
    unsigned long long value = val;

    if (value == 0 && bufsz > 0)
    {
        buf[0] = '0';
        len = 1;
    }
    else while (len < bufsz && value)
    {
        buf[len] = '0' + (value % 10);
        value /= 10;
        len += 1;
    }

    return len;
}



static int print_hex_ll(char *buf, int bufsz, unsigned long value, unsigned int flags)
{
    int len = 0;
    unsigned long long val = value;
    const char *lut = s_hexchars;
    char hex = 'x';

    if (flags & CAPITALIZED)
    {
        hex = 'X';
        lut = s_HEXCHARS;
    }

    if (0 == val && bufsz > 0)
    {
        buf[0] = '0';
        len = 1;
    }
    else while (val && len < bufsz)
    {
        buf[len] = lut[val & 0xF];
        val >>= 4;
        len += 1;
    }

    if ((flags & ALTERNATE_FORM) && (len + 2 <= bufsz))
    {
        buf[len] = hex;
        buf[len + 1] = '0';
        len += 2;
    }
    return len;
}





static void print_num_lld(char **bufptr, fs_size *left, int *ret,
    long long value, int minw, int precision, unsigned int flags)
{
    char tmp[DEC_BUFSIZE];
    unsigned long long abs_val;
    int len;
    unsigned int flags2 = flags;

    flags2 = flags2 | ((value < 0) << VALUE_NEG_POS);
    flags2 = flags2 | ((value == 0) << VALUE_ZERO_POS);

    if (value < 0)
        abs_val = -value;
    else
        abs_val = value;

    len = print_decimal_ll(tmp, DEC_BUFSIZE, abs_val);
    print_num_pad(bufptr, left, ret, minw, precision, flags2,  
        tmp, len
    );
}




static void print_num_llu(char **bufptr, fs_size *left, int *ret,
    unsigned long long value, int minw, int precision, unsigned int flags)
{
    char tmp[DEC_BUFSIZE];
    int len;
    unsigned int flags2 = flags;
    flags2 = flags2 | ((value == 0) << VALUE_ZERO_POS);

    len = print_decimal_ll(tmp, DEC_BUFSIZE, value);
    print_num_pad(bufptr, left, ret, minw, precision, flags2,  
        tmp, len
    );
}



static void print_num_llx(char **bufptr, fs_size *left, int *ret,
    unsigned long long value, int minw, int precision, unsigned int flags)
{
    char tmp[DEC_BUFSIZE];
    int len;
    unsigned int flags2 = flags;
    flags2 = flags2 | ((value == 0) << VALUE_ZERO_POS);

    len = print_hex_ll(tmp, DEC_BUFSIZE, value, flags2);
    print_num_pad(bufptr, left, ret, minw, precision, flags2, 
        tmp, len
    );
}





#endif /* FS_C99 */




/* outbuf is assumed to have a size of HEX_BUFSIZE */
static int print_hex_bytes(char *outbuf, const void *ptr, unsigned int flags)
{
    const char *lut = s_hexchars;
    char hex = 'x';
    union {
        fs_u8 bytes[sizeof(ptr)];
        const void *ptr;
    } cvt;
    unsigned int i = 0;


    cvt.ptr = ptr;
    if (flags & CAPITALIZED)
    {
        lut = s_HEXCHARS;
        hex = 'X';
    }


    fs_endian_host_to_little(cvt.bytes, 1, sizeof(ptr));
    for (; i < sizeof(ptr)*2; i += 2)
    {
        outbuf[i] = lut[0xF & cvt.bytes[i/2]];              /* lower 4 bits */
        outbuf[i + 1] = lut[0xF & (cvt.bytes[i/2] >> 4)];   /* upper 4 bits */
    }


    /* outbuf   :   "00102030" */
    if (!(flags & ALTERNATE_FORM))
    {
        /* trim the leading zeros */
        while (i > 1 && (outbuf[i - 1] == '0')) 
            i -= 1;
    }
    /* 0x */
    outbuf[i] = hex;
    outbuf[i + 1] = '0';
    i += 2;
    return i;
}


static void print_ptr(char **bufptr, fs_size *left, int *ret, 
    const void *ptr, int minw, int precision, unsigned int flags)
{
    char hexbuf[HEX_BUFSIZE];
    int len;

    if (NULL == ptr)
    {
        print_str(bufptr, left, ret, 
            "(nil)", minw, precision, flags
        );
        return;
    }

    len = print_hex_bytes(hexbuf, ptr, flags);
    print_num_pad(bufptr, left, ret, minw, 
        precision, flags, 
        hexbuf, len
    );
}








int fs_snprintf(char *buf, fs_size bufsz, const char *fmt, ...)
{
    int ret;
    va_list args;
    va_start(args, fmt);
    ret = fs_vsnprintf(buf, bufsz, fmt, args);
    va_end(args);
    return ret;
}

int fs_vsnprintf(char *buf, fs_size bufsz, const char *fmt, va_list ap)
{
    unsigned int flags = 0;
    int minw;
    int precision;
    int l_count;
    int conv;
    int ret = 0;
    const char *fmtptr = fmt;
    char *bufptr = buf;
    fs_size left = bufsz;


    while (*fmtptr)
    {
        /* copy raw string */
        while (*fmtptr && '%' != *fmtptr)
        {
            if (left > 1)
            {
                *bufptr = *fmtptr;
                bufptr += 1;
                fmtptr += 1;
                left -= 1;
            }
            else fmtptr += 1;
            ret += 1;
        }

        if (0 == *fmtptr) break; /* null character */
        fmtptr += 1;


        flags = 0;
        minw = 0;
        l_count = 0;
        precision = 1;


        /* get flags */
        for (;;fmtptr += 1)
        {
            switch (*fmtptr)
            {
            case ' ': flags = flags | SPACE; break;
            case '0': flags = flags | ZEROPAD; break;
            case '+': flags = flags | PLUS; break;
            case '-': flags = flags | PAD_RIGHT; break;
            case '#': flags = flags | ALTERNATE_FORM; break;
            default: goto done_flags;
            }
        }
done_flags:

        /* get width */
        if ('*' == *fmtptr)
        {
            fmtptr += 1; /* skips '*' */
            minw = va_arg(ap, int);
            if (minw < 0)
            {
                flags = flags | PAD_RIGHT;
                minw = -minw;
            }
        }
        else while (is_number(*fmtptr))
        {
            minw = minw * 10 + (*fmtptr) - '0';
            fmtptr += 1;
        }


        /* get precision */
        if ('.' == *fmtptr)
        {
            fmtptr += 1; /* skip '.' */
            flags = flags | PRECISION_PROVIDED;
            precision = 0;
            if ('*' == *fmtptr)
            {
                precision = va_arg(ap, int);
                if (precision < 0)
                {
                    precision = 0;
                }
            }
            else while (is_number(*fmtptr))
            {
                precision = precision * 10 + (*fmtptr) - '0';
                fmtptr += 1;
            }
        }


        /* get length */
        if ('l' == *fmtptr)
        {
            fmtptr += 1; /* skip 'l' */
            l_count = 1;
            if ('l' == *fmtptr)
            {
                fmtptr += 1;
                l_count = 2;
            }
        }

        conv = *fmtptr;
        if (conv) fmtptr += 1;

        switch (conv)
        {
        case 'I':
        case 'D':
            flags = flags | CAPITALIZED;
            goto id_fmt; /* suppress implicit fallthrough warning */
        case 'i':
        case 'd':
id_fmt:
            if (l_count == 0)
                print_num_ld(&bufptr, &left, &ret, 
                    va_arg(ap, int), minw, precision, flags
                );
            else if (l_count == 1)
                print_num_ld(&bufptr, &left, &ret, 
                    va_arg(ap, long), minw, precision, flags
                );
#ifdef FS_C99
            else if (l_count == 2)
            {
                print_num_lld(&bufptr, &left, &ret,
                    va_arg(ap, long long), minw, precision, flags
                );
            }
#endif /* FS_C99 */
            break;


        case 'u':
            if (l_count == 0)
                print_num_lu(&bufptr, &left, &ret, 
                    va_arg(ap, unsigned int), minw, precision, flags
                );
            else if (l_count == 1)
                print_num_lu(&bufptr, &left, &ret, 
                    va_arg(ap, unsigned long), minw, precision, flags
                );
#ifdef FS_C99
            else if (l_count == 2)
                print_num_llu(&bufptr, &left, &ret, 
                    va_arg(ap, unsigned long long), minw, precision, flags
                );
#endif /* FS_C99 */
            break;


        case 'X': flags = flags | CAPITALIZED; goto x_fmt;
        case 'x':
x_fmt:
            if (l_count == 0)
                print_num_lx(&bufptr, &left, &ret, 
                    va_arg(ap, unsigned int), minw, precision, flags
                );
            else if (l_count == 1)
                print_num_lx(&bufptr, &left, &ret, 
                    va_arg(ap, unsigned long), minw, precision, flags
                );
#ifdef FS_C99
            else if (l_count == 2)
                print_num_llx(&bufptr, &left, &ret, 
                    va_arg(ap, unsigned long long), minw, precision, flags
                );
#endif /* FS_C99 */
            break;


        case 'S': flags = flags | CAPITALIZED; goto s_fmt;
        case 's':
s_fmt:
            print_str(&bufptr, &left, &ret, 
                va_arg(ap, const char *), minw, precision, flags
            );
            break;


        case 'C': flags = flags | CAPITALIZED; goto c_fmt;
        case 'c':
c_fmt:
            print_chr(&bufptr, &left, &ret, 
                va_arg(ap, int), minw, flags
            );
            break;

#ifndef FREESTANDING_TRULY
        case 'm':
            print_str(&bufptr, &left, &ret, 
                strerror(errno), minw, precision, flags
            );
            break;
#endif /* FREESTANDING_TRULY */

        case 'p':
            print_ptr(&bufptr, &left, &ret,
                va_arg(ap, const void *), minw, precision, flags
            );
            break;

        case '%':
            print_chr(&bufptr, &left, &ret, 
                '%', minw, flags
            );
            break;

        case 'n':
            {
                int *p = va_arg(ap, int*);
                *p = ret;
            }
            break;





        default: 
        case 0: break;
        }
    }


    if (left > 0)
        *bufptr = 0;
    else bufptr[-1] = 0;
    return ret;
}











#ifdef SNPRINTF_TEST



#include <stdio.h>
#include <string.h>
#include <stdlib.h>



#ifndef FS_C99
#  error "at least C99 is required for testing code"
#endif /* FS_C99 */




/** do tests */
#define DOTEST(bufsz, result, retval, ...) do { \
    char buf[bufsz]; \
    int r;\
    printf("[INFO]: Now test %s\n", #__VA_ARGS__); \
    r=fs_snprintf(buf, sizeof(buf), __VA_ARGS__); \
    if(r != retval || strcmp(buf, result) != 0) { \
        printf("  [ERROR]: test(%s) was '%s':%d\n", \
                ""#bufsz", "#result", "#retval", "#__VA_ARGS__, \
                buf, r); \
        exit(1); \
    } \
    r=snprintf(buf, sizeof(buf), __VA_ARGS__); \
    if(r != retval || strcmp(buf, result) != 0) { \
        printf("  [WARNING]: test(%s) differs with system, '%s':%d\n", \
                ""#bufsz", "#result", "#retval", "#__VA_ARGS__, \
                buf, r); \
    } \
    printf("  test(\"%s\":%d) passed\n", buf, r); \
} while(0);




/** test program */
int main(void)
{
    /* bufsize, expectedstring, expectedretval, snprintf arguments */
    DOTEST(1024, "hello", 5, "hello");
    DOTEST(1024, "h", 1, "h");
    /* warning from gcc for format string, but it does work
     *   * DOTEST(1024, "", 0, ""); */

    DOTEST(3, "he", 5, "hello");
    DOTEST(1, "", 7, "%d", 7823089);

    /* test positive numbers */
    DOTEST(1024, "0", 1, "%d", 0);
    DOTEST(1024, "1", 1, "%d", 1);
    DOTEST(1024, "9", 1, "%d", 9);
    DOTEST(1024, "15", 2, "%d", 15);
    DOTEST(1024, "ab15cd", 6, "ab%dcd", 15);
    DOTEST(1024, "167", 3, "%d", 167);
    DOTEST(1024, "7823089", 7, "%d", 7823089);
    DOTEST(1024, " 12", 3, "%3d", 12);
    DOTEST(1024, "012", 3, "%.3d", 12);
    DOTEST(1024, "012", 3, "%3.3d", 12);
    DOTEST(1024, "012", 3, "%03d", 12);
    DOTEST(1024, " 012", 4, "%4.3d", 12);
    DOTEST(1024, "", 0, "%.0d", 0);

    /* test negative numbers */
    DOTEST(1024, "-1", 2, "%d", -1);
    DOTEST(1024, "-12", 3, "%3d", -12);
    DOTEST(1024, " -2", 3, "%3d", -2);
    DOTEST(1024, "-012", 4, "%.3d", -12);
    DOTEST(1024, "-012", 4, "%3.3d", -12);
    DOTEST(1024, "-012", 4, "%4.3d", -12);
    DOTEST(1024, " -012", 5, "%5.3d", -12);
    DOTEST(1024, "-12", 3, "%03d", -12);
    DOTEST(1024, "-02", 3, "%03d", -2);
    DOTEST(1024, "-15", 3, "%d", -15);
    DOTEST(1024, "-7307", 5, "%d", -7307);
    DOTEST(1024, "-12  ", 5, "%-5d", -12);
    DOTEST(1024, "-00012", 6, "%-.5d", -12);

    /* test + and space flags */
    DOTEST(1024, "+12", 3, "%+d", 12);
    DOTEST(1024, " 12", 3, "% d", 12);

    /* test %u */
    DOTEST(1024, "12", 2, "%u", 12);
    DOTEST(1024, "0", 1, "%u", 0);
    DOTEST(1024, "4294967295", 10, "%u", 0xffffffff);

    /* test %x */
    DOTEST(1024, "0", 1, "%x", 0);
    DOTEST(1024, "c", 1, "%x", 12);
    DOTEST(1024, "12ab34cd", 8, "%x", 0x12ab34cd);
    DOTEST(1024, "ABCD", 4, "%X", 0xABCD);
    DOTEST(1024, "0XABCD", 6, "%#X", 0xABCD);


    /* test %llu, %lld */
    DOTEST(1024, "18446744073709551615", 20, "%llu",
            (long long)0xffffffffffffffff);
    DOTEST(1024, "-9223372036854775808", 20, "%lld",
            (long long)0x8000000000000000);
    DOTEST(1024, "9223372036854775808", 19, "%llu",
            (long long)0x8000000000000000);

    /* test %s */
    DOTEST(1024, "hello", 5, "%s", "hello");
    DOTEST(1024, "     hello", 10, "%10s", "hello");
    DOTEST(1024, "hello     ", 10, "%-10s", "hello");
    DOTEST(1024, "he", 2, "%.2s", "hello");
    DOTEST(1024, "  he", 4, "%4.2s", "hello");
    DOTEST(1024, "   h", 4, "%4.2s", "h");

    /* test %c */
    DOTEST(1024, "a", 1, "%c", 'a');
    /* warning from gcc for format string, but it does work
     *     DOTEST(1024, "    a", 5, "%5c", 'a');
     *         DOTEST(1024, "a", 1, "%.0c", 'a'); */

    /* test %n */
    {
        const char *expect = "hello  ";
        char buf[1024];
        int fs_x, x, fs_ret, ret;

        printf("[INFO]: Now test %%n\n");
        fs_ret = fs_snprintf(buf, sizeof buf, "hello  %n", &fs_x);
        if (strcmp(buf, expect) != 0)
        {
            printf("  [ERROR]: mismatched strings: (fs:'%s' != expect:'%s')\n", buf, expect);
            exit(1);
        }
        ret = snprintf(buf, sizeof buf, "hello  %n", &x);
        if (ret != fs_ret)
        {
            printf("  [ERROR]: mismatched return value (fs:%d != std:%d)\n", fs_ret, ret);
            exit(1);
        }
        if (fs_x != x)
        {
            printf("  [ERROR]: '%%n' failed: (fs:%d != std:%d)", fs_x, x);
            exit(1);
        }
        printf("  test %%n passed\n");
    }

    /* test %m */
#ifndef FREESTANDING_TRULY
#  ifdef __GNUC__
    errno = 0;
    {
        const char *es = strerror(errno);
        DOTEST(1024, es, strlen(es), "%m");
    }
#  endif
#endif

    /* test %p */
    DOTEST(1024, "0x10", 4, "%p", (void*)0x10);
    DOTEST(1024, "(nil)", 5, "%p", (void*)0x0);

    /* test %% */
    DOTEST(1024, "%", 1, "%%");

    /* test %f */
    DOTEST(1024, "0.000000", 8, "%f", 0.0);
    DOTEST(1024, "0.00", 4, "%.2f", 0.0);
    /* differs, "-0.00" DOTEST(1024, "0.00", 4, "%.2f", -0.0); */
    DOTEST(1024, "234.00", 6, "%.2f", 234.005);
    DOTEST(1024, "8973497.1246", 12, "%.4f", 8973497.12456);
    DOTEST(1024, "-12.000000", 10, "%f", -12.0);
    DOTEST(1024, "6", 1, "%.0f", 6.0);

    DOTEST(1024, "6", 1, "%g", 6.0);
    DOTEST(1024, "6.1", 3, "%g", 6.1);
    DOTEST(1024, "6.15", 4, "%g", 6.15);

    /* These format strings are from the code of NSD, Unbound, ldns */

    DOTEST(1024, "abcdef", 6, "%s", "abcdef");
    DOTEST(1024, "005", 3, "%03u", 5);
    DOTEST(1024, "12345", 5, "%03u", 12345);
    DOTEST(1024, "5", 1, "%d", 5);
    DOTEST(1024, "(nil)", 5, "%p", NULL);
    DOTEST(1024, "12345", 5, "%ld", (long)12345);
    DOTEST(1024, "12345", 5, "%lu", (long)12345);
    DOTEST(1024, "       12345", 12, "%12u", (unsigned)12345);
    DOTEST(1024, "12345", 5, "%u", (unsigned)12345);
    DOTEST(1024, "12345", 5, "%llu", (unsigned long long)12345);
    DOTEST(1024, "12345", 5, "%x", 0x12345);
    DOTEST(1024, "12345", 5, "%llx", (long long)0x12345);
    DOTEST(1024, "012345", 6, "%6.6d", 12345);
    DOTEST(1024, "012345", 6, "%6.6u", 12345);
    DOTEST(1024, "1234.54", 7, "%g", 1234.54);
    DOTEST(1024, "123456789.54", 12, "%.12g", 123456789.54);
    DOTEST(1024, "3456789123456.54", 16, "%.16g", 3456789123456.54);
    /* %24g does not work with 24 digits, not enough accuracy,
     *   * the first 16 digits are correct */
    DOTEST(1024, "12345", 5, "%3.3d", 12345);
    DOTEST(1024, "000", 3, "%3.3d", 0);
    DOTEST(1024, "001", 3, "%3.3d", 1);
    DOTEST(1024, "012", 3, "%3.3d", 12);
    DOTEST(1024, "-012", 4, "%3.3d", -12);
    DOTEST(1024, "he", 2, "%.2s", "hello");
    DOTEST(1024, "helloworld", 10, "%s%s", "hello", "world");
    DOTEST(1024, "he", 2, "%.*s", 2, "hello");
    DOTEST(1024, "  hello", 7, "%*s", 7, "hello");
    DOTEST(1024, "hello  ", 7, "%*s", -7, "hello");
    DOTEST(1024, "0", 1, "%c", '0'); 
    DOTEST(1024, "A", 1, "%c", 'A'); 
    DOTEST(1024, "", 1, "%c", 0); 
    DOTEST(1024, "\010", 1, "%c", 8); 
    DOTEST(1024, "%", 1, "%%"); 
    DOTEST(1024, "0a", 2, "%02x", 0x0a); 
    DOTEST(1024, "bd", 2, "%02x", 0xbd); 
    DOTEST(1024, "12", 2, "%02ld", (long)12); 
    DOTEST(1024, "02", 2, "%02ld", (long)2); 
    DOTEST(1024, "02", 2, "%02u", (unsigned)2); 
    DOTEST(1024, "765432", 6, "%05u", (unsigned)765432); 
    DOTEST(1024, "10.234", 6, "%0.3f", 10.23421); 
    DOTEST(1024, "123456.234", 10, "%0.3f", 123456.23421); 
    DOTEST(1024, "123456789.234", 13, "%0.3f", 123456789.23421); 
    DOTEST(1024, "123456.23", 9, "%.2f", 123456.23421); 
    DOTEST(1024, "123456", 6, "%.0f", 123456.23421); 
    DOTEST(1024, "0123", 4, "%.4x", 0x0123); 
    DOTEST(1024, "00000123", 8, "%.8x", 0x0123); 
    DOTEST(1024, "ffeb0cde", 8, "%.8x", 0xffeb0cde); 
    DOTEST(1024, " 987654321", 10, "%10lu", (unsigned long)987654321); 
    DOTEST(1024, "   987654321", 12, "%12lu", (unsigned long)987654321); 
    DOTEST(1024, "987654321", 9, "%i", 987654321); 
    DOTEST(1024, "-87654321", 9, "%i", -87654321); 
    DOTEST(1024, "hello           ", 16, "%-16s", "hello"); 
    DOTEST(1024, "                ", 16, "%-16s", ""); 
    DOTEST(1024, "a               ", 16, "%-16s", "a"); 
    DOTEST(1024, "foobarfoobar    ", 16, "%-16s", "foobarfoobar"); 
    DOTEST(1024, "foobarfoobarfoobar", 18, "%-16s", "foobarfoobarfoobar"); 

    /* combined expressions */
    DOTEST(1024, "foo 1.0 size 512 edns", 21,
            "foo %s size %d %s%s", "1.0", 512, "", "edns");
    DOTEST(15, "foo 1.0 size 5", 21,
            "foo %s size %d %s%s", "1.0", 512, "", "edns");
    DOTEST(1024, "packet 1203ceff id", 18,
            "packet %2.2x%2.2x%2.2x%2.2x id", 0x12, 0x03, 0xce, 0xff);
    DOTEST(1024, "/tmp/testbound_123abcd.tmp", 26, "/tmp/testbound_%u%s%s.tmp", 123, "ab", "cd");

    return 0;
}
#endif /* SNPRINTF_TEST */




