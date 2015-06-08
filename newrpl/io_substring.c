/*
 * Copyright (c) 2008-2016 Stefan Krah. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include "mpdecimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
//#include <locale.h>
#include "bits.h"
#include "constants.h"
#include "memory.h"
#include "typearith.h"
#include "io.h"


/* This file contains functions for decimal <-> string conversions, including
   PEP-3101 formatting for numeric types. */

/* This file duplicates the functionality of io.c, but uses sub strings,
 * not necessarily terminated in a null. This is needed to avoid copying text
 * during compilation.
*/

static inline int
_mpd_strneq(const char *s, const char *l, const char *u, size_t n)
{
    while (--n != SIZE_MAX) {
        if (*s != *l && *s != *u) {
            return 0;
        }
        s++; u++; l++;
    }

    return 1;
}

/*
 * Partially verify a numeric string of the form:
 *
 *     [cdigits][.][cdigits][eE][+-][edigits]
 *
 * If successful, return a pointer to the location of the first
 * relevant coefficient digit. This digit is either non-zero or
 * part of one of the following patterns:
 *
 *     ["0\x00", "0.\x00", "0.E", "0.e", "0E", "0e"]
 *
 * The locations of a single optional dot or indicator are stored
 * in 'dpoint' and 'exp'.
 *
 * The end of the string is stored in 'end'. If an indicator [eE]
 * occurs without trailing [edigits], the condition is caught
 * later by strtoexp().
 */

#define internal_isdigit(c) ( ((c)>='0') && ((c)<='9'))




static const char *
scan_dpoint_exp2(const char *s, const char *send, const char **dpoint, const char **exp,
                const char **end)
{
    const char *coeff = NULL;

    *dpoint = NULL;
    *exp = NULL;
    for (; s != send ; s++) {
        switch (*s) {
        case '.':
            if (*dpoint != NULL || *exp != NULL)
                return NULL;
            *dpoint = s;
            break;
        case 'E': case 'e':
            if (*exp != NULL)
                return NULL;
            *exp = s;
            if (*(s+1) == '+' || *(s+1) == '-')
                s++;
            break;
        default:
            if (!internal_isdigit((uchar)*s))
                return NULL;
            if (coeff == NULL && *exp == NULL) {
                if (*s == '0') {
                    if (!internal_isdigit((uchar)*(s+1)))
                        if (!(*(s+1) == '.' &&
                              internal_isdigit((uchar)*(s+2))))
                            coeff = s;
                }
                else {
                    coeff = s;
                }
            }
            break;

        }
    }

    *end = s;
    return coeff;
}





/* scan the payload of a NaN */
static const char *
scan_payload(const char *s, const char **end)
{
    const char *coeff;

    while (*s == '0')
        s++;
    coeff = s;

    while ( (*s>='0') && (*s<='9'))
        s++;
    *end = s;

    return (*s == '\0') ? coeff : NULL;
}


/*
 * Scan 'len' words. The most significant word contains 'r' digits,
 * the remaining words are full words. Skip dpoint. The string 's' must
 * consist of digits and an optional single decimal point at 'dpoint'.
 */
static void
string_to_coeff(mpd_uint_t *data, const char *s, const char *dpoint, int r,
                size_t len)
{
    int j;

    if (r > 0) {
        data[--len] = 0;
        for (j = 0; j < r; j++, s++) {
            if (s == dpoint) s++;
            data[len] = 10 * data[len] + (*s - '0');
        }
    }

    while (--len != SIZE_MAX) {
        data[len] = 0;
        for (j = 0; j < MPD_RDIGITS; j++, s++) {
            if (s == dpoint) s++;
            data[len] = 10 * data[len] + (*s - '0');
        }
    }
}



long mpd_strtossize2(const char *number,const char *numend,char **endptr,int base)
{
    int digit,neg=0,countdigit=0;
    long long result=0;
    char *start=(char *)number;

    // SKIP INITIAL BLANKS
    while(((*number==' ') || (*number=='\t')) && (number<numend)) ++number;

    if(number>=numend) {
        errno=EINVAL;
        return 0;
    }


    if(*number=='+') ++number;
    else if(*number=='-') { neg=1; ++number; }

    if(number>=numend) {
        errno=EINVAL;
        return 0;
    }

    if(base==16) {
        if(*number=='0') {
            if((number[1]=='x')||(number[1]=='X')) number+=2;
        }
    }




while(number<numend) {
    digit=-1;
    if((*number>='0')&&(*number<='9')) digit=*number-'0';
    if((*number>='a') &&(*number<='z')) digit=*number-'a'+10;
    if((*number>='A')&&(*number<='Z')) digit=*number-'A'+10;
    if(digit<0 || digit>=base) {

        if(!countdigit) number=start;
        if(neg) result=-result;

        if(result<INT_MIN) {
            errno=ERANGE;
            result=INT_MIN;
        }

            if(result>INT_MAX) {
                errno=ERANGE;
                result=INT_MAX;
            }
    //SET END POINTER AND RETURN
        if((endptr)) *endptr=(char *)number;
        return result;
    }
    ++countdigit;
    ++number;
    result=result*base+digit;
}

if(!countdigit) number=start;

if(neg) result=-result;
if(result<INT_MIN) {
    errno=ERANGE;
    result=INT_MIN;
}

    if(result>INT_MAX) {
        errno=ERANGE;
        result=INT_MAX;
    }
//SET END POINTER AND RETURN
if((endptr)) *endptr=(char *)number;
return result;
}




static mpd_ssize_t
strtoexp2(const char *s,const char *send)
{
    char *end;
    mpd_ssize_t retval;

    errno = 0;
    retval = mpd_strtossize2(s, send,&end, 10);
    if (errno == 0 && !(end == send))
        errno = EINVAL;

    return retval;
}




/* convert a character string to a decimal */
void
mpd_qset_string2(mpd_t *dec, const char *s, const char *send, const mpd_context_t *ctx,
                uint32_t *status)
{
    mpd_ssize_t q, r, len;
    const char *coeff, *end;
    const char *dpoint = NULL, *exp = NULL;
    size_t digits;
    uint8_t sign = MPD_POS;

    mpd_set_flags(dec, 0);
    dec->len = 0;
    dec->exp = 0;

    /* sign */
    if (*s == '+') {
        s++;
    }
    else if (*s == '-') {
        mpd_set_negative(dec);
        sign = MPD_NEG;
        s++;
    }

    if (_mpd_strneq(s, "nan", "NAN", 3)) { /* NaN */
        s += 3;
        mpd_setspecial(dec, sign, MPD_NAN);
        if (*s == '\0')
            return;
        /* validate payload: digits only */
        if ((coeff = scan_payload(s, &end)) == NULL)
            goto conversion_error;
        /* payload consists entirely of zeros */
        if (*coeff == '\0')
            return;
        digits = end - coeff;
        /* prec >= 1, clamp is 0 or 1 */
        if (digits > (size_t)(ctx->prec-ctx->clamp))
            goto conversion_error;
    } /* sNaN */
    else if (_mpd_strneq(s, "snan", "SNAN", 4)) {
        s += 4;
        mpd_setspecial(dec, sign, MPD_SNAN);
        if (*s == '\0')
            return;
        /* validate payload: digits only */
        if ((coeff = scan_payload(s, &end)) == NULL)
            goto conversion_error;
        /* payload consists entirely of zeros */
        if (*coeff == '\0')
            return;
        digits = end - coeff;
        if (digits > (size_t)(ctx->prec-ctx->clamp))
            goto conversion_error;
    }
    else if (_mpd_strneq(s, "inf", "INF", 3)) {
        s += 3;
        if (*s == '\0' || _mpd_strneq(s, "inity", "INITY", 6)) {
            /* numeric-value: infinity */
            mpd_setspecial(dec, sign, MPD_INF);
            return;
        }
        goto conversion_error;
    }
    else {
        /* scan for start of coefficient, decimal point, indicator, end */
        if ((coeff = scan_dpoint_exp2(s, send, &dpoint, &exp, &end)) == NULL)
            goto conversion_error;

        /* numeric-value: [exponent-part] */
        if (exp) {
            /* exponent-part */
            end = exp; exp++;
            dec->exp = strtoexp2(exp,send);
            if (errno) {
                if (!(errno == ERANGE &&
                     (dec->exp == MPD_SSIZE_MAX ||
                      dec->exp == MPD_SSIZE_MIN)))
                    goto conversion_error;
            }
        }

            digits = end - coeff;
        if (dpoint) {
            size_t fracdigits = end-dpoint-1;
            if (dpoint > coeff) digits--;

            if (fracdigits > MPD_MAX_PREC) {
                goto conversion_error;
            }
            if (dec->exp < MPD_SSIZE_MIN+(mpd_ssize_t)fracdigits) {
                dec->exp = MPD_SSIZE_MIN;
            }
            else {
                dec->exp -= (mpd_ssize_t)fracdigits;
            }
        }
        if (digits > MPD_MAX_PREC) {
            goto conversion_error;
        }
        if (dec->exp > MPD_EXP_INF) {
            dec->exp = MPD_EXP_INF;
        }
        if (dec->exp == MPD_SSIZE_MIN) {
            dec->exp = MPD_SSIZE_MIN+1;
        }
    }

    _mpd_idiv_word(&q, &r, (mpd_ssize_t)digits, MPD_RDIGITS);

    len = (r == 0) ? q : q+1;
    if (len == 0) {
        goto conversion_error; /* GCOV_NOT_REACHED */
    }
    if (!mpd_qresize(dec, len, status)) {
        mpd_seterror(dec, MPD_Malloc_error, status);
        return;
    }
    dec->len = len;

    string_to_coeff(dec->data, coeff, dpoint, (int)r, len);

    mpd_setdigits(dec);
    mpd_qfinalize(dec, ctx, status);
    return;

conversion_error:
    /* standard wants a positive NaN */
    mpd_seterror(dec, MPD_Conversion_syntax, status);
}

