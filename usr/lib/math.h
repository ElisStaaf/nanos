#ifndef __NANOS_USERSPLIB_MATH_H
#define __NANOS_USERSPLIB_MATH_H

#if defined(__GNUC__)
#   define    HUGE_VAL     __builtin_huge_val()
#   define    HUGE_VALF    __builtin_huge_valf()
#   define    HUGE_VALL    __builtin_huge_vall()
#   define    NAN          __builtin_nanf("0x7fc00000")
#else
#   define    HUGE_VAL     1e500
#   define    HUGE_VALF    1e50f
#   define    HUGE_VALL    1e5000L
#   define    NAN          __nan()
#endif

#define INFINITY    HUGE_VALF

float sqrtspf(float);
float sinspf(float);
float cosspf(float);
float tanspf(float);
float tanhspf(float);
float asinspf(float);
float acosspf(float);
float atanspf(float);

#endif
