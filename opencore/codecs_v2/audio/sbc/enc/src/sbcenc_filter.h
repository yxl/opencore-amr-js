/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
#ifndef		__FILTER__
#define		__FILTER__

#include "oscl_types.h"

void analysis_filter_4(analysis_filter_t *, sbc_t *);
void analysis_filter_8(analysis_filter_t *, sbc_t *);

#ifdef ARM

__inline Int  FMULT(Int a, Int b)
{
    Int temp;
    __asm
    {
        smulwb temp, a, b
        mov   temp, temp, asl #1

    }
    return (temp);
}

__inline Int  FMULT_1(Int a, Int b)
{
    Int tmp1;
    Int tmp2;
    __asm
    {
        smull tmp1, tmp2, a, b
        mov   tmp2, tmp2, asl #2
        mov   tmp1, tmp1, lsr #30
        orr   tmp2, tmp1, tmp2
    }
    return (tmp2);
}

__inline Int  FMULT_2(Int a, Int b)
{
    Int tmp1;
    Int tmp2;
    __asm
    {
        smull tmp1, tmp2, a, b
        mov   tmp2, tmp2, asl #16
        mov   tmp1, tmp1, lsr #16
        orr   tmp2, tmp1, tmp2
    }
    return (tmp2);
}

#elif defined (NO_64BIT_SUPPORT)

#define MASKLOW16 0xFFFF

__inline Int MULT(Int a, Int b, Int shift)
{
    Int   flag = 0;     /* carries negative sign, if any  */
    Int   c;
    Int   high32bits;
    Int   med_32bits_a;
    Int   med_32bits_b;
    UInt  low32bits;
    UInt  Result_low32bits;
    Int   carry;
    UInt  temp;

    if (a < 0)
    {
        a = -a;
        flag ^= 1;
    }
    if (b < 0)
    {
        b = -b;
        flag ^= 1;
    }

    low32bits = a & MASKLOW16;             /* temporary storage */
    Result_low32bits = b & MASKLOW16;      /* temporary storage */

    high32bits   = ((a >> 16) * (b >> 16));               /* Top 32 bits of resulting 64 */
    med_32bits_a = ((a >> 16) * Result_low32bits);        /* Midlle 32 bits (16 to 48) of resulting 64 */
    med_32bits_b = ((Int)low32bits * (b >> 16));
    low32bits    = ((UInt)low32bits * Result_low32bits);/* Lower 32 bits of resulting 64 */

    /* carry on evaluation */
    temp = (UInt)(med_32bits_a << 16) + (UInt)(med_32bits_b << 16);
    carry = ((temp < (UInt)(med_32bits_a << 16)) || (temp < (UInt)(med_32bits_b << 16)));

    Result_low32bits  =  temp + low32bits;
    carry += ((Result_low32bits < temp) || (Result_low32bits < low32bits));

    high32bits = high32bits + (med_32bits_a >> 16) + (med_32bits_b >> 16) + carry;

    c = ((high32bits << (32 - shift)) | (Result_low32bits >> (shift)));

    return (flag ? -c : c);
}

#define FMULT(a,b)    MULT(a, b, 15)
#define FMULT_1(a,b)  MULT(a, b, 30)
#define FMULT_2(a,b)  MULT(a, b, 16)


#else

#define FMULT(a, b)	(Int)(((Int64)(a) *  (b)) >> 15) //Output is kept in Q15 format.
#define FMULT_1(a, b)	(Int)(((Int64)(a) *  (b)) >> 30) //Output is kept in Q15 format.
#define FMULT_2(a, b)	(Int)(((Int64)(a) *  (b)) >> 16)

#endif

#endif /* __FILTER__ */
